/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#include <papyro/annotator.h>
#include <papyro/decorator.h>
#include <papyro/dispatcher.h>
#include <papyro/dispatcher_p.h>
#include <papyro/utils.h>
#include <utopia2/qt/cache.h>

#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QWidget>

#include <QDebug>

Q_DECLARE_METATYPE(Spine::AnnotationHandle);


QDataStream & operator >> (QDataStream & str, QList< Spine::AnnotationHandle > & annotationList)
{
    qFatal("QList< Spine::AnnotationHandle > cannot be serialised");
    return str;
}

namespace Papyro
{

    /// DispatcherPrivate /////////////////////////////////////////////////////////////////////////


    class DispatcherPrivate
    {
    public:
        DispatcherPrivate() : derivedCache(":Dispatcher.derivedCache") {}

        // Cache for derived annotations
        QMap< QString, QList< Spine::AnnotationHandle > > cachedAnnotations;
        // Cache for derived annotations
        Utopia::Cache< QList< Spine::AnnotationHandle > > derivedCache;

        // Extensions
        QList< boost::shared_ptr< Annotator > > lookups;
        QList< Decorator * > decorators;

        // Engine
        QList< DispatchEngine * > liveEngines;
        QList< DispatchEngine * > deadEngines;

        // Mutual exclusion
        QMutex mutex;

        // Default Id for new sessions
        QString defaultSessionId;
    };


    /// LookupRunnable ////////////////////////////////////////////////////////////////////////////


    LookupRunnable::LookupRunnable(DispatcherPrivate * d, DispatchEngine * engine, Spine::DocumentHandle document, boost::shared_ptr< Annotator > annotator, const QStringList & terms)
        : QObject(0), QRunnable(), d(d), engine(engine), document(document), annotator(annotator), terms(terms)
    {}

    LookupRunnable::~LookupRunnable()
    {}

    void LookupRunnable::run()
    {
        static uint seed = 1;
        static QMutex seedLock;
        seedLock.lock();
        qsrand(seed);
        ++seed;
        seedLock.unlock();

        if (!engine->detached()) {
            std::set< Spine::AnnotationHandle > annotations(annotator->lookup(document, unicodeFromQString(terms.first())));
            foreach (Spine::AnnotationHandle annotation, annotations) {
                // Give each a cssId
                annotation->setProperty("session:cssId", unicodeFromQString(QString("result-") + QString("000000000000%1").arg(qrand()).right(8)));
                // Make sure they know where they came from
                annotation->setProperty("session:origin", "explore");
                annotation->setProperty("session:exploredTerm", unicodeFromQString(terms.first()));
                // Decorate
                foreach (Decorator * decorator, d->decorators) {
                    if (engine->detached()) { return; }
                    foreach (Spine::CapabilityHandle capability, decorator->decorate(annotation)) {
                        annotation->addCapability(capability);
                    }
                }
                if (engine->detached()) { return; }
                emit annotationFound(annotation);
            }
        }
    }


    /// DispatchEngine ////////////////////////////////////////////////////////////////////////////


    DispatchEngine::DispatchEngine(Dispatcher * dispatcher,
                                   DispatcherPrivate * dispatcherPrivate,
                                   Spine::DocumentHandle document,
                                   const QStringList & terms)
        : QThread(dispatcher), d(dispatcherPrivate), mutex(QMutex::Recursive), cancelled(false), document(document)
    {
        qRegisterMetaType< Spine::AnnotationHandle >();

        // Connect up engine's signals
        connect(this, SIGNAL(annotationFound(Spine::AnnotationHandle)),
                dispatcher, SLOT(onAnnotationFound(Spine::AnnotationHandle)));
        connect(this, SIGNAL(finished()), dispatcher, SIGNAL(finished()));

        queue += terms;
    }

    DispatchEngine::~DispatchEngine()
    {}

    void DispatchEngine::cancel()
    {
        QMutexLocker guard(&mutex);

        // Cancel all queued runnables
        cancelled = true;
    }

    void DispatchEngine::detach()
    {
        QMutexLocker guard(&mutex);
        // Disconnect all signals
        disconnect(dispatcher(), SLOT(onAnnotationFound(Spine::AnnotationHandle)));
        disconnect(dispatcher(), SIGNAL(finished()));

        cancel();
    }

    bool DispatchEngine::detached() const
    {
        QMutexLocker guard(&mutex);
        return cancelled;
    }

    Dispatcher * DispatchEngine::dispatcher()
    {
        QMutexLocker guard(&mutex);
        return qobject_cast< Dispatcher * >(parent());
    }

    void DispatchEngine::run()
    {
        //qDebug() << "[" << QDateTime::currentDateTime().toString().toStdString().c_str() << "]" << this << "started.";

        // The engine will wait for a condition (i.e. more data to process, or stop processing)
        // whereupon it will do as it is told. Its tasks complete with the following priorities:
        //    1. Stop processing and clean up (highest priority)
        //    2. Perform a term lookup (lowest priority)
        // This list of tasks may grow as and when the Dispatcher takes on further responsiblities.

        // Local queue of tasks
        QMap< boost::shared_ptr< Annotator >, QQueue< QString > > annotationTasks;
        QMap< QString, QList< Spine::AnnotationHandle > > derivedCacheTmp;

        // Populate task list
        if (queue.size() > 0)
        {
            QMutexLocker guard(&d->mutex);

            QListIterator< boost::shared_ptr< Annotator > > l_iter(d->lookups);
            while (l_iter.hasNext())
            {
                LookupRunnable * runnable = new LookupRunnable(d, this, document, l_iter.next(), queue);
                connect(runnable, SIGNAL(annotationFound(Spine::AnnotationHandle)), this, SIGNAL(annotationFound(Spine::AnnotationHandle)));
                threadPool.start(runnable);
            }
        }

        // FIXME Update the cache
        /*
        d->mutex.lock();
        QMapIterator< QString, QList< Spine::AnnotationHandle > > c_iter(derivedCacheTmp);
        while (c_iter.hasNext())
        {
            c_iter.next();

            QString cacheId = c_iter.key();
            QString term = cacheId;
            term.replace(QRegExp("^[0-9]*\\s*"), "");

            d->derivedCache.put(c_iter.value(), cacheId);
        }
        d->mutex.unlock();
        */

        //emit finished();
        //qDebug() << "[" << QDateTime::currentDateTime().toString().toStdString().c_str() << "]" << this << "stopped.";

        // Wait for lookups to finish
        threadPool.waitForDone();

        // Inform the system we've finished
        emit finished();

        document.reset();
    }


    /// Dispatcher ////////////////////////////////////////////////////////////////////////////////


    Dispatcher::Dispatcher(QObject * parent)
        : QObject(parent)
    {
        d = new DispatcherPrivate();

        initialise();
    }

    Dispatcher::~Dispatcher()
    {
        clear();

        // Join engines!
        {
            QListIterator< DispatchEngine * > e_iter(d->deadEngines);
            while (e_iter.hasNext()) {
                e_iter.next()->wait();
            }
        }
        {
            QListIterator< DispatchEngine * > e_iter(d->liveEngines);
            while (e_iter.hasNext()) {
                e_iter.next()->wait();
            }
        }

        delete d;
    }

    void Dispatcher::clear()
    {
        // Remove previous engine
        if (!d->liveEngines.empty())
        {
            foreach (DispatchEngine * engine, d->liveEngines) {
                engine->detach();
            }
            d->deadEngines.append(d->liveEngines);
            d->liveEngines.clear();
            emit finished();

            // Tell views of clear
            emit cleared();
        }
    }

    QString Dispatcher::defaultSessionId() const
    {
        return d->defaultSessionId;
    }

    void Dispatcher::initialise()
    {
        // Set default session Id
        d->defaultSessionId = "default";
    }

    void Dispatcher::lookupOLD(Spine::DocumentHandle document, const QString & term, bool cancelCurrent)
    {
        QStringList terms;
        terms.append(term);
        lookupOLD(document, terms, cancelCurrent);
    }

    void Dispatcher::lookupOLD(Spine::DocumentHandle document, const QStringList & terms, bool cancelCurrent)
    {
        if (cancelCurrent) {
            // Clear current state
            clear();
        }

        // Create new engine
        DispatchEngine * engine = new DispatchEngine(this, d, document, terms);
        d->liveEngines.append(engine);

        // Nudge the engine to start processing
        engine->start();

        emit started();
    }

    void Dispatcher::onAnnotationFound(Spine::AnnotationHandle annotation)
    {
        emit annotationFound(annotation);
    }

    void Dispatcher::setDecorators(const QList< Decorator * > & decorators)
    {
        d->decorators = decorators;
    }

    void Dispatcher::setDefaultSessionId(const QString & sid)
    {
        d->defaultSessionId = sid;
    }

    void Dispatcher::setLookups(const QList< boost::shared_ptr< Annotator > > & lookups)
    {
        d->lookups = lookups;
    }

}
