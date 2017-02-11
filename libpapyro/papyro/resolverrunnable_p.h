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

#ifndef ATHENAEUM_RESOLVERRUNNABLE_P_H
#define ATHENAEUM_RESOLVERRUNNABLE_P_H

#include <papyro/citation.h>
#include <papyro/resolver.h>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <QMutex>
#include <QVariant>

namespace Athenaeum
{

    class Resolver;

    typedef std::map< int, std::vector< boost::shared_ptr< Resolver > > > _ResolverMap;

    boost::shared_ptr< _ResolverMap > get_resolvers();

    class ResolverRunnablePrivate
    {
    public:
        ResolverRunnablePrivate();

        Resolver::Purposes purposes;
        Athenaeum::CitationHandle citation;
        Spine::DocumentHandle document;
        boost::shared_ptr< _ResolverMap > resolvers;
        QList< boost::shared_ptr< Resolver > > queue;
        boost::shared_ptr< Resolver > running;
        bool cancelled;
        QMutex mutex;
    }; // class ResolverRunnablePrivate

} // namespace Athenaeum

#endif // ATHENAEUM_RESOLVERRUNNABLE_P_H
