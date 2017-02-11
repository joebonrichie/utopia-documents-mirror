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

#ifndef UTOPIA_SINGLETON_H
#define UTOPIA_SINGLETON_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QThreadStorage>

namespace Utopia
{

    template< class V, class T, class S = T >
    struct global_singleton_traits
    {
        typedef V value_type;
        typedef T transport_type;
        typedef S storage_type;
        static storage_type & storage()
        {
            static storage_type storage;
            return storage;
        }
    };

    template< class V, class T, class S = T >
    struct threadlocal_singleton_traits
    {
        typedef V value_type;
        typedef T transport_type;
        typedef S storage_type;
        static storage_type & storage()
        {
            static QThreadStorage< storage_type * > storage;
            if (!storage.hasLocalData()) {
                storage.setLocalData(new storage_type);
            }
            return *storage.localData();
        }
    };

    template< class V >
    struct permanant_singleton_traits : public global_singleton_traits< V, boost::shared_ptr< V > > {};

    template< class V >
    struct temporary_singleton_traits : public global_singleton_traits< V, boost::shared_ptr< V >, boost::weak_ptr< V > > {};

    template< class V >
    struct permanant_threadlocal_singleton_traits : public threadlocal_singleton_traits< V, boost::shared_ptr< V > > {};

    template< class V >
    struct temporary_threadlocal_singleton_traits : public threadlocal_singleton_traits< V, boost::shared_ptr< V >, boost::weak_ptr< V > > {};




    template< typename Traits >
    class SingletonBase
    {
    public:
        typedef typename Traits::transport_type value_type;

        static typename Traits::transport_type instance()
        {
            typename Traits::transport_type transport = Traits::storage();
            if (!Traits::storage()) {
                Traits::storage() = transport = typename Traits::transport_type(new typename Traits::value_type);
            }
            return transport;
        }
    }; // class Singleton

    template< typename V > class Singleton : public SingletonBase< struct permanant_singleton_traits< V > > {};
    template< typename V > class ThreadLocalSingleton : public SingletonBase< struct permanant_threadlocal_singleton_traits< V > > {};
    template< typename V > class TempSingleton : public SingletonBase< struct temporary_singleton_traits< V > > {};
    template< typename V > class TempThreadLocalSingleton : public SingletonBase< struct temporary_threadlocal_singleton_traits< V > > {};

}

#endif // UTOPIA_SINGLETON_H
