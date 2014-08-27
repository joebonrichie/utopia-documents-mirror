/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <athenaeum/resolverrunnable_p.h>
#include <athenaeum/resolverrunnable.h>
#include <athenaeum/resolver.h>

#include <boost/weak_ptr.hpp>

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




    ResolverRunnable::ResolverRunnable(const QModelIndex & index, const QVariantMap & metadata)
        : QObject(0), QRunnable(), d(new ResolverRunnablePrivate)
    {
        d->index = index;
        d->metadata = metadata;
        d->resolvers = get_resolvers();
    }

    ResolverRunnable::~ResolverRunnable()
    {
        // Nothing
    }

    void ResolverRunnable::run()
    {
        emit started();

        _ResolverMap::const_iterator iter(d->resolvers->begin());
        _ResolverMap::const_iterator end(d->resolvers->end());
        for (; iter != end; ++iter) {
            foreach (boost::shared_ptr< Resolver > resolver, iter->second) {
                QMapIterator< QString, QVariant > iter(resolver->resolve(d->metadata));
                while (iter.hasNext()) {
                    iter.next();
                    if (iter.value().isValid()) {
                        d->metadata[iter.key()] = iter.value();
                    } else {
                        d->metadata.remove(iter.key());
                    }
                }
            }
        }

        emit completed();
        emit completed(d->index, d->metadata);
    }

} // namespace Athenaeum
