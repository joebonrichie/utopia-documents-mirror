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

#include <papyro/resolverqueue_p.h>
#include <papyro/resolverqueue.h>
#include <papyro/citations.h>
#include <papyro/bibliography.h>
#include <papyro/abstractbibliography.h>

#include <QDateTime>
#include <QFile>
#include <QPointer>
#include <QRunnable>
#include <QThreadPool>
#include <QUrl>

#include <QDebug>

namespace Athenaeum
{

    ResolverJob::ResolverJob(CitationHandle citation,
                             Resolver::Purposes purposes,
                             Spine::DocumentHandle document)
        : citation(citation), purposes(purposes), document(document)
    {}




    ResolverQueueRunnable::ResolverQueueRunnable(ResolverQueuePrivate * d)
        : d(d), cancelled(false), mutex(QMutex::Recursive), documentManager(Papyro::DocumentManager::instance())
    {
        // Collect all the resolvers in order
        _ResolverMap::const_iterator iter(d->resolvers.begin());
        _ResolverMap::const_iterator end(d->resolvers.end());
        for (; iter != end; ++iter) {
            foreach (boost::shared_ptr< Resolver > resolver, iter->second) {
                resolvers.push_back(resolver);
            }
        }
    }

    void ResolverQueueRunnable::cancel()
    {
        QMutexLocker guard(&mutex);
        cancelled = true;
    }

    bool ResolverQueueRunnable::isCancelled()
    {
        QMutexLocker guard(&mutex);
        return cancelled;
    }

    void ResolverQueueRunnable::run()
    {
        if (!isCancelled() && d) {
            ResolverJob next = d->next();
            if (CitationHandle citation = next.citation) {
                // not already running etc, then run
                AbstractBibliography::State state = citation->field(Citation::StateRole).value< AbstractBibliography::State >();
                QDateTime dateResolved = citation->field(Citation::DateResolvedRole).toDateTime();
                if (!dateResolved.isValid() && state == AbstractBibliography::IdleState) {
                    citation->setField(Citation::StateRole, QVariant::fromValue(AbstractBibliography::BusyState));

                    if (!next.document) {
                        QUrl originatingUri(citation->field(Citation::OriginatingUriRole).toUrl());
                        if (originatingUri.isLocalFile()) {
                            QFile originatingFile(originatingUri.toLocalFile());
                            if (originatingFile.open(QIODevice::ReadOnly)) {
                                next.document = documentManager->open(&originatingFile);
                            }
                        }
                    }

                    QVariantMap qCitation = citation->toMap();
                    QVariantMap provenance = qCitation["provenance"].toMap();
                    QVariantList sources = provenance["sources"].toList();
                    QVariantList qCitations;
                    if (sources.isEmpty()) {
                        qCitations << qCitation;
                    } else {
                        qCitations = sources;
                    }

                    while (!resolvers.isEmpty() && !isCancelled()) {
                        running = resolvers.takeFirst();

                        bool shouldStop = false;
                        if (running->purposes() & next.purposes) {
                            qCitations = running->resolve(qCitations, next.document);
                            foreach (QVariant variant, qCitations) {
                                if (variant.toMap().value("_action").toString() == "stop") {
                                    shouldStop = true;
                                    break;
                                }
                            }
                        }

                        running.reset();

                        // Cancel this pipeline if asked to by this resolver
                        if (shouldStop) {
                            break;
                        }
                    }

                    if (!isCancelled()) {

                        qCitation = Papyro::flatten(qCitations);

                        citation->updateFromMap(qCitation);
                        citation->setField(Citation::StateRole, QVariant::fromValue(AbstractBibliography::IdleState));
                        citation->setField(Citation::DateResolvedRole, QDateTime::currentDateTime());
                    }
                }
            }
        }

       // deleteLater();
    }




    ResolverQueuePrivate::ResolverQueuePrivate(Bibliography * bibliography, QObject * parent)
        : QObject(parent), bibliography(bibliography), mutex(QMutex::Recursive)
    {
        connect(bibliography, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(onDataChanged(const QModelIndex &, const QModelIndex &)));
        connect(bibliography, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        connect(bibliography, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
                this, SLOT(onRowsAboutToBeRemoved(const QModelIndex &, int, int)));

        // Populate resolver list
        foreach (Resolver * resolver, Utopia::instantiateAllExtensions< Resolver >()) {
            resolvers[resolver->weight()].push_back(boost::shared_ptr< Resolver >(resolver));
        }
    }

    ResolverQueuePrivate::~ResolverQueuePrivate()
    {
        cancel();
        threadPool.waitForDone();
    }

    void ResolverQueuePrivate::cancel()
    {
        emit (cancelled());
    }

    ResolverJob ResolverQueuePrivate::next()
    {
        // Get the next citation from the top of the stack
        QMutexLocker lock(&mutex);
        if (!stack.isEmpty()) {
            return stack.takeLast();
        }
        return ResolverJob();
    }

    void ResolverQueuePrivate::queue(CitationHandle citation, int priority)
    {
        QMutexLocker lock(&mutex);
        stack.append(ResolverJob(citation));
        ResolverQueueRunnable * runnable = new ResolverQueueRunnable(this);
        connect(this, SIGNAL(cancelled()), runnable, SLOT(cancel()), Qt::DirectConnection);
        runnable->setAutoDelete(true);
        threadPool.start(runnable, priority);
    }

    void ResolverQueuePrivate::onDataChanged(const QModelIndex & from, const QModelIndex & to)
    {
    }

    void ResolverQueuePrivate::onRowsInserted(const QModelIndex & parent, int from, int to)
    {
        for (int row = from; row <= to; ++row) {
            CitationHandle citation(bibliography->data(bibliography->index(row, 0, parent), Citation::ItemRole).value< CitationHandle >());
            QDateTime dateResolved = citation->field(Citation::DateResolvedRole).toDateTime();
            if (!dateResolved.isValid()) {
                queue(citation);
            }
        }
    }

    void ResolverQueuePrivate::onRowsAboutToBeRemoved(const QModelIndex & parent, int from, int to)
    {
        for (int row = from; row <= to; ++row) {
            CitationHandle citation(bibliography->data(bibliography->index(row, 0, parent), Citation::ItemRole).value< CitationHandle >());
            unqueue(citation);
        }
    }

    void ResolverQueuePrivate::unqueue(CitationHandle citation)
    {
    }




    ResolverQueue::ResolverQueue(Bibliography * bibliography, QObject * parent)
        : QObject(parent), d(new ResolverQueuePrivate(bibliography, this))
    {}

    ResolverQueue::~ResolverQueue()
    {}

    void ResolverQueue::cancel()
    {
        d->cancel();
    }

} // namespace Athenaeum
