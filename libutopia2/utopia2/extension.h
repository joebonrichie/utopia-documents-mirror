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

#ifndef Utopia_EXTENSION_H
#define Utopia_EXTENSION_H

#include <utopia2/config.h>

#include <utopia2/extensionfactory.h>
#include <map>
#include <set>
#include <string>

#include <boost/shared_ptr.hpp>

#include <iostream>

namespace Utopia
{

    template< class ExtensionAPI >
    class Extension
    {
    public:
        // Inspection
        static std::set< std::string > registeredNames()
        {
            std::set< std::string > names;

            typename std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > >::iterator iter(get().begin());
            typename std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > >::iterator end(get().end());
            for (; iter != end; ++iter) {
                names.insert(iter->first);
            }

            return names;
        }

        template< class ExtensionFactoryImpl >
        static void registerExtension(const std::string & name, ExtensionFactoryImpl * factory)
        {
            //std::cerr << "Registering extention " << name << " (" << get().size() << ") " << &get() << std::endl;
            get()[name] = boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > >(factory);
        }

        template< class ExtensionImpl >
        static void registerExtension(const std::string & name)
        {
            Extension< ExtensionAPI >::registerExtension(name, new ExtensionFactory< ExtensionImpl, ExtensionAPI >());
        }

        static ExtensionAPI * instantiateExtension(const std::string & name, bool singleton = false)
        {
            if (get().find(name) != get().end())
            {
                return get()[name]->instantiate(singleton);
            }
            else
            {
                return 0;
            }
        }

        static ExtensionAPI * instantiateExtensionOnce(const std::string & name)
        {
            return Extension< ExtensionAPI >::instantiateExtension(name, true);
        }

        static std::set< ExtensionAPI * > instantiateAllExtensions(bool singleton = false)
        {
            std::set< ExtensionAPI * > extensions;

            typename std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > >::iterator iter(get().begin());
            typename std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > >::iterator end(get().end());
            for (; iter != end; ++iter)
            {
                extensions.insert(iter->second->instantiate(singleton));
            }
            return extensions;
        }

        static std::set< ExtensionAPI * > instantiateAllExtensionsOnce()
        {
            return Extension< ExtensionAPI >::instantiateAllExtensions(true);
        }

        static void unregisterExtension(const std::string & name)
        {
            get().erase(name);
        }

        static std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > > & get()
        {
#if defined(_WIN32)
            // Windows requires this to be a static member
            return _registry;
#else
            // OSX requires this to be a static local variable in class scope
            static std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > > _reg;
            return _reg;
#endif // _WIN32
        }

    protected:

#if defined(_WIN32)
        static std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > > _registry;
#endif // _WIN32
    };


#if defined(_WIN32)
    template< class ExtensionAPI >
    std::map< std::string, boost::shared_ptr< ExtensionFactoryBase< ExtensionAPI > > > Extension< ExtensionAPI >::_registry;
#endif // _WIN32

    template< class ExtensionImpl, class ExtensionDecl >
    void registerExtension(const std::string & name)
    {
        Extension< ExtensionDecl >::template registerExtension< ExtensionImpl >(name);
    }

    template< class ExtensionImpl >
    void registerExtension(const std::string & name)
    {
        registerExtension< ExtensionImpl, typename ExtensionImpl::API >(name);
    }

    template< class ExtensionFactoryImpl, class ExtensionFactoryDecl >
    void registerExtension(const std::string & name, ExtensionFactoryImpl * factory)
    {
        Extension< ExtensionFactoryDecl >::registerExtension(name, factory);
    }

    template< class ExtensionFactoryImpl >
    void registerExtension(const std::string & name, ExtensionFactoryImpl * factory)
    {
        registerExtension< ExtensionFactoryImpl, typename ExtensionFactoryImpl::API >(name, factory);
    }

    template< class ExtensionAPI >
    ExtensionAPI * instantiateExtension(const std::string & name, bool singleton = false)
    {
        return Extension< ExtensionAPI >::instantiateExtension(name, singleton);
    }

    template< class ExtensionAPI >
    ExtensionAPI * instantiateExtensionOnce(const std::string & name)
    {
        return instantiateExtension< ExtensionAPI >(name, true);
    }

    template< class ExtensionAPI >
    std::set< ExtensionAPI * > instantiateAllExtensions(bool singleton = false)
    {
        return Extension< ExtensionAPI >::instantiateAllExtensions(singleton);
    }

    template< class ExtensionAPI >
    std::set< ExtensionAPI * > instantiateAllExtensionsOnce()
    {
        return instantiateAllExtensions< ExtensionAPI >(true);
    }

    template< class ExtensionAPI >
    std::set< std::string > registeredExtensionNames()
    {
        return Extension< ExtensionAPI >::registeredNames();
    }

} // namespace Utopia

#ifdef _WIN32
#  define UTOPIA_DECLARE_EXTENSION_CLASS(pkg, fqn) extern template class ::Utopia::Extension< fqn >;
#  define UTOPIA_DEFINE_EXTENSION_CLASS(fqn) template class ::Utopia::Extension< fqn >;
#else
#  define UTOPIA_DECLARE_EXTENSION_CLASS(pkg, fqn)
#  define UTOPIA_DEFINE_EXTENSION_CLASS(fqn) template class ::Utopia::Extension< fqn >;
#endif

#define UTOPIA_REGISTER_EXTENSION_NAMED(fqn, name) ::Utopia::registerExtension< fqn >(name);
#define UTOPIA_REGISTER_EXTENSION(fqn) UTOPIA_REGISTER_EXTENSION_NAMED(fqn, #fqn)

#define UTOPIA_REGISTER_EXTENSION_FACTORY_NAMED(fqn, name, factory) ::Utopia::registerExtension< fqn >(name, factory);
#define UTOPIA_REGISTER_EXTENSION_FACTORY(fqn) UTOPIA_REGISTER_EXTENSION_FACTORY_NAMED(fqn, #fqn, factory)

#define UTOPIA_REGISTER_TYPED_EXTENSION_NAMED(fqndecl, fqn, name) ::Utopia::registerExtension< fqn, fqndecl >(name);
#define UTOPIA_REGISTER_TYPED_EXTENSION(fqndecl, fqn) UTOPIA_REGISTER_TYPED_EXTENSION_NAMED(fqndecl, fqn, #fqn)

#define UTOPIA_REGISTER_TYPED_EXTENSION_FACTORY_NAMED(fqndecl, fqn, name, factory) ::Utopia::registerExtension< fqn, fqndecl >(name, factory);
#define UTOPIA_REGISTER_TYPED_EXTENSION_FACTORY(fqndecl, fqn) UTOPIA_REGISTER_TYPED_EXTENSION_FACTORY_NAMED(fqndecl, fqn, #fqn, factory)

#endif // Utopia_EXTENSION_H
