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

#ifndef ATHENAEUM_RESOLVERQUEUE_P_H
#define ATHENAEUM_RESOLVERQUEUE_P_H

#include <papyro/documentmanager.h>
#include <papyro/citation.h>
#include <papyro/resolver.h>
#include <boost/shared_ptr.hpp>

#include <QModelIndex>
#include <QObject>
#include <QMutex>
#include <QPointer>
#include <QRunnable>
#include <QThreadPool>

#include <map>
#include <vector>

namespace Athenaeum
{

    class Bibliography;

    typedef std::map< int, std::vector< boost::shared_ptr< Resolver > > > _ResolverMap;

    class ResolverJob
    {
    public:
        ResolverJob(CitationHandle citation = CitationHandle(),
                    Resolver::Purposes purposes = Resolver::AllPurposes,
                    Spine::DocumentHandle document = Spine::DocumentHandle());

        CitationHandle citation;
        Resolver::Purposes purposes;
        Spine::DocumentHandle document;
    }; // class ResolverJob




    class ResolverQueuePrivate : public QObject
    {
        Q_OBJECT

    public:
        ResolverQueuePrivate(Bibliography * bibliography, QObject * parent = 0);
        ~ResolverQueuePrivate();

        ResolverJob next();
        void queue(CitationHandle citation, int priority = -1);
        void unqueue(CitationHandle citation);

    signals:
        void cancelled();

    public slots:
        void cancel();
        void onDataChanged(const QModelIndex &, const QModelIndex &);
        void onRowsInserted(const QModelIndex &, int, int);
        void onRowsAboutToBeRemoved(const QModelIndex &, int, int);

    public:
        Bibliography * bibliography;
        QList< ResolverJob > stack;
        QMutex mutex;
        _ResolverMap resolvers;
        QThreadPool threadPool;
    }; // class ResolverQueuePrivate




    class ResolverQueueRunnable : public QObject, public QRunnable
    {
        Q_OBJECT

    public:
        ResolverQueueRunnable(ResolverQueuePrivate * d);

        bool isCancelled();
        void run();

    public slots:
        void cancel();

    protected:
        QPointer< ResolverQueuePrivate > d;
        QList< boost::shared_ptr< Resolver > > resolvers;
        boost::shared_ptr< Resolver > running;
        bool cancelled;
        QMutex mutex;
        boost::shared_ptr< Papyro::DocumentManager > documentManager;

    }; // class ResolverQueueRunnable

} // namespace Athenaeum

#endif // ATHENAEUM_RESOLVERQUEUE_P_H
