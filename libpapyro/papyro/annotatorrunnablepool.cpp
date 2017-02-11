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

#include <papyro/annotatorrunnable.h>
#include <papyro/annotatorrunnablepool.h>
#include <papyro/annotatorrunnablepool_p.h>

#include <QEventLoop>

namespace Papyro
{

    /// SyncPointEmitter ////////////////////////////////////////////////////////////////

    SyncPointEmitter::SyncPointEmitter(QObject * parent)
        : QObject(parent)
    {}

    void SyncPointEmitter::emitSyncPoint()
    {
        Q_EMIT synced();
    }




    /// AnnotatorRunnablePool ///////////////////////////////////////////////////////////

    AnnotatorRunnablePool::AnnotatorRunnablePool(QObject * parent)
        : QObject(parent), d(new AnnotatorRunnablePoolPrivate)
    {
        d->queued = 0;
        d->running = 0;
        d->finished = 0;
        d->futureQueued = 0;
    }

    AnnotatorRunnablePool::~AnnotatorRunnablePool()
    {
        skip();

        delete d;
    }

    bool AnnotatorRunnablePool::isActive()
    {
        return d->running + d->queued > 0;
    }

    void AnnotatorRunnablePool::onStarted()
    {
        if (d->running == 0)
        {
            Q_EMIT started();
        }

        --d->queued;
        ++d->running;
    }

    void AnnotatorRunnablePool::onFinished()
    {
        --d->running;
        ++d->finished;

        if (d->running + d->queued == 0)
        {
            // FIXME only Q_EMIT this when not future queues exist?
            Q_EMIT finished();

            // Dequeue futures until something is queued, or nothing left
            bool stop = false;
            int size = d->futureQueues.size();
            while (--size >= 0 && !d->futureQueues.isEmpty() && !stop)
            {
                // Let people know this pool just synced
                Q_EMIT synced();

                QList< QPair< AnnotatorRunnable *, int > > futureQueue(d->futureQueues.takeFirst());
                SyncPointEmitter * emitter = d->futureEmitters.takeFirst();

                // Queue next set of runnables
                if (!futureQueue.isEmpty())
                {
                    QListIterator< QPair< AnnotatorRunnable *, int > > f_iter(futureQueue);
                    while (f_iter.hasNext())
                    {
                        QPair< AnnotatorRunnable *, int > info(f_iter.next());
                        _start(info.first, info.second);
                        --d->futureQueued;
                    }
                    stop = true;
                }

                if (emitter)
                {
                    emitter->emitSyncPoint();
                    delete emitter;
                }
            }
        }
    }

    void AnnotatorRunnablePool::skip()
    {
        // Skip all runnables
        QList< AnnotatorRunnable * > runnables = findChildren< AnnotatorRunnable * >();
        QListIterator< AnnotatorRunnable * > runnable(runnables);
        while (runnable.hasNext())
        {
            runnable.next()->skip();
        }

        // Remove future queues
        QListIterator< QList< QPair< AnnotatorRunnable *, int > > > futureQueues(d->futureQueues);
        while (futureQueues.hasNext())
        {
            QListIterator< QPair< AnnotatorRunnable *, int > > futureQueue(futureQueues.next());
            while (futureQueue.hasNext())
            {
                delete futureQueue.next().first;
                --d->futureQueued;
            }
        }
        d->futureQueues.clear();

        // Emit all pending sync points
        QListIterator< SyncPointEmitter * > emitters(d->futureEmitters);
        while (emitters.hasNext())
        {
            SyncPointEmitter * emitter = emitters.next();
            if (emitter)
            {
                emitter->emitSyncPoint();
                delete emitter;
            }
        }
        d->futureEmitters.clear();

        // Sync for further runnables
        sync();
    }

    void AnnotatorRunnablePool::start(QList< AnnotatorRunnable * > runnables, int priority)
    {
        QListIterator< AnnotatorRunnable * > runnable(runnables);
        while (runnable.hasNext())
        {
            start(runnable.next(), priority);
        }
    }

    void AnnotatorRunnablePool::_start(AnnotatorRunnable * runnable, int priority)
    {
        connect(runnable, SIGNAL(started()), this, SLOT(onStarted()));
        connect(runnable, SIGNAL(finished()), this, SLOT(onFinished()));
        d->threadPool.start(runnable, priority);
        ++d->queued;
    }

    void AnnotatorRunnablePool::start(AnnotatorRunnable * runnable, int priority)
    {
        runnable->setParent(this);

        if (d->futureQueues.isEmpty())
        {
            _start(runnable, priority);
        }
        else
        {
            d->futureQueues.back().append(qMakePair(runnable, priority));
            ++d->futureQueued;
        }
    }

    void AnnotatorRunnablePool::sync()
    {
        // Sync without emitter
        sync(0, 0);
    }

    void AnnotatorRunnablePool::sync(const QObject * receiver, const char * method, Qt::ConnectionType type)
    {
        // If reciever/method pair is specified, then sync (or Q_EMIT synced() immediately if queue is empty)
        if (receiver && method)
        {
            SyncPointEmitter * emitter = new SyncPointEmitter(this);
            connect(emitter, SIGNAL(synced()), receiver, method, type);

            if (d->running + d->queued == 0)
            {
                emitter->emitSyncPoint();
                delete emitter;
            }
            else
            {
                d->futureQueues.append(QList< QPair< AnnotatorRunnable *, int > >());
                d->futureEmitters.append(emitter);
            }
        }
        // Add a new sync point if required
        else if (d->running + d->queued > 0 && (d->futureQueues.isEmpty() || !d->futureQueues.back().isEmpty()))
        {
            d->futureQueues.append(QList< QPair< AnnotatorRunnable *, int > >());
            d->futureEmitters.append(0);
        }
    }

    void AnnotatorRunnablePool::waitForDone()
    {
        d->threadPool.waitForDone();
    }

} // namespace Papyro
