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

#ifndef ATHENAEUM_RESOLVERRUNNABLE_H
#define ATHENAEUM_RESOLVERRUNNABLE_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <papyro/resolver.h>
#  include <spine/Document.h>
#endif
#include <papyro/citation.h>
#include <QModelIndex>
#include <QObject>
#include <QRunnable>
#include <QVariant>

namespace Athenaeum
{

    class ResolverRunnablePrivate;
    class ResolverRunnable : public QObject, public QRunnable
    {
        Q_OBJECT

    public:
        ResolverRunnable(Athenaeum::CitationHandle citation, Resolver::Purposes purposes, Spine::DocumentHandle document = Spine::DocumentHandle());
        ~ResolverRunnable();

        void run();

        static ResolverRunnable * resolve(Athenaeum::CitationHandle citation,
                                          QObject * obj,
                                          const char * method,
                                          Resolver::Purposes purposes = Resolver::AllPurposes,
                                          Spine::DocumentHandle document = Spine::DocumentHandle());

        static ResolverRunnable *  identify(Athenaeum::CitationHandle citation, QObject * obj, const char * method, Spine::DocumentHandle document = Spine::DocumentHandle())
        { return resolve(citation, obj, method, Resolver::Identify, document); }
        static ResolverRunnable *  dereference(Athenaeum::CitationHandle citation, QObject * obj, const char * method, Spine::DocumentHandle document = Spine::DocumentHandle())
        { return resolve(citation, obj, method, Resolver::Dereference, document); }
        static ResolverRunnable *  expand(Athenaeum::CitationHandle citation, QObject * obj, const char * method, Spine::DocumentHandle document = Spine::DocumentHandle())
        { return resolve(citation, obj, method, Resolver::Expand, document); }

    signals:
        void cancelled();
        void completed();
        void completed(Athenaeum::CitationHandle citation);
        void failed();
        void started();

    public slots:
        virtual void cancel();

    protected:
        ResolverRunnablePrivate * d;
    }; // class ResolverRunnable

} // namespace Athenaeum

#endif // ATHENAEUM_RESOLVERRUNNABLE_H
