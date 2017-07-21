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

#include <papyro/resolverrunnable_p.h>
#include <papyro/resolverrunnable.h>
#include <papyro/abstractbibliography.h>
#include <papyro/resolver.h>
#include <papyro/citations.h>

#include <boost/weak_ptr.hpp>

#include <QDateTime>
#include <QThreadPool>

#include <QDebug>

namespace Athenaeum
{

    // Singleton list of resolver plugins

    boost::shared_ptr< _ResolverMap > get_resolvers()
    {
        static boost::weak_ptr< _ResolverMap > singleton;

        boost::shared_ptr< _ResolverMap > shared(singleton.lock());
        if (singleton.expired()) {
            shared = boost::shared_ptr< _ResolverMap >(new _ResolverMap);
            singleton = shared;

            // Populate resolver list
            foreach (Resolver * resolver, Utopia::instantiateAllExtensions< Resolver >()) {
                (*shared)[resolver->weight()].push_back(boost::shared_ptr< Resolver >(resolver));
            }
        }
        return shared;
    }




    ResolverRunnablePrivate::ResolverRunnablePrivate()
        : cancelled(false), mutex(QMutex::Recursive)
    {}




    ResolverRunnable::ResolverRunnable(Athenaeum::CitationHandle citation, Resolver::Purposes purposes, Spine::DocumentHandle document)
        : QObject(0), QRunnable(), d(new ResolverRunnablePrivate)
    {
        d->purposes = purposes;
        d->citation = citation;
        d->document = document;
        d->resolvers = get_resolvers();
    }

    ResolverRunnable::~ResolverRunnable()
    {
        // Nothing
    }

    void ResolverRunnable::cancel()
    {
        QMutexLocker guard(&d->mutex);
        d->cancelled = true;
        d->queue.clear();
        // Signal the resolver to cancel, if possible
        if (d->running) {
            d->running->cancel();
        }
        d->running.reset();
    }

    ResolverRunnable * ResolverRunnable::resolve(Athenaeum::CitationHandle citation,
                                                 QObject * obj,
                                                 const char * method,
                                                 Resolver::Purposes purposes,
                                                 Spine::DocumentHandle document)
    {
        static QThreadPool threadPool;
        static bool set = false;
        if (!set) {
            threadPool.setMaxThreadCount(40);
            set = true;
        }

        ResolverRunnable * resolverRunnable = new ResolverRunnable(citation, purposes, document);
        connect(resolverRunnable, SIGNAL(completed(Athenaeum::CitationHandle)), obj, method);
        //QThreadPool::globalInstance()->start(resolverRunnable);
        threadPool.start(resolverRunnable);
        return resolverRunnable;
    }

    void ResolverRunnable::run()
    {
        emit started();

        // Work on a copy of the provided metadata
        Athenaeum::CitationHandle citation(d->citation);
        citation->setField(Citation::StateRole, QVariant::fromValue(AbstractBibliography::BusyState));

        QVariantMap qCitation = citation->toMap();
        QVariantMap provenance = qCitation.value("provenance").toMap();
        QVariantList sources = provenance["sources"].toList();
        QVariantList qCitations;
        if (sources.isEmpty()) {
            qCitations << qCitation;
        } else {
            qCitations = sources;
        }

        {
            // Reset management information
            QMutexLocker guard(&d->mutex);
            d->running.reset();
            d->queue.clear();

            // Collect all the resolvers in order
            _ResolverMap::const_iterator iter(d->resolvers->begin());
            _ResolverMap::const_iterator end(d->resolvers->end());
            for (; iter != end; ++iter) {
                foreach (boost::shared_ptr< Resolver > resolver, iter->second) {
                    d->queue.push_back(resolver);
                }
            }
        }

        // Run resolvers over metadata in order, while there are new resolvers to run
        d->mutex.lock();

        while (!d->queue.isEmpty() && !d->cancelled) {
            d->running = d->queue.takeFirst();
            d->mutex.unlock();

            bool shouldStop = false;
            if (d->running->purposes() & d->purposes) {
                qCitations = d->running->resolve(qCitations, d->document);
                foreach (QVariant variant, qCitations) {
                    if (variant.toMap().value("_action").toString() == "stop") {
                        shouldStop = true;
                        break;
                    }
                }
            }

            d->mutex.lock();
            d->running.reset();

            // Cancel this pipeline if asked to by this resolver
            if (shouldStop) {
                d->cancelled = true;
            }
        }
        bool isCancelled = d->cancelled;
        d->mutex.unlock();

        qCitation = Papyro::flatten(qCitations);

        citation->updateFromMap(qCitation);
        citation->setField(Citation::StateRole, QVariant::fromValue(AbstractBibliography::IdleState));
        citation->setField(Citation::DateResolvedRole, QDateTime::currentDateTime());
        emit completed();
        if (isCancelled) {
            emit cancelled();
        } else {
            qRegisterMetaType< Athenaeum::CitationHandle >("Athenaeum::CitationHandle");
            emit completed(citation);
        }
    }

} // namespace Athenaeum
