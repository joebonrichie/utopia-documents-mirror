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

#ifndef BOOST_PP_IS_ITERATING

#ifndef Utopia_EXTENSIONFACTORY_H
#define Utopia_EXTENSIONFACTORY_H

#include <utopia2/config.h>

#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <set>
#include <string>

#ifndef EXTENSIONFACTORY_MAX_PARAMS
#define EXTENSIONFACTORY_MAX_PARAMS 2
#endif

#define EXTENSIONFACTORY_templateparameterdecl(z, n, data) class T ## n = void

namespace Utopia
{

    template< class ExtensionAPI >
    class ExtensionFactoryBase
    {
    public:
        typedef ExtensionAPI API;
        virtual ExtensionAPI * instantiate(bool singleton = false) const = 0;

    protected:
        mutable boost::scoped_ptr< ExtensionAPI > _singleton;
    };

    template< class Extension, typename ExtensionDecl, BOOST_PP_ENUM(EXTENSIONFACTORY_MAX_PARAMS, EXTENSIONFACTORY_templateparameterdecl, ~) >
    class ExtensionFactory;

    template< class Extension, typename ExtensionDecl, class Callback, BOOST_PP_ENUM(EXTENSIONFACTORY_MAX_PARAMS, EXTENSIONFACTORY_templateparameterdecl, ~)>
    class ExtensionCallbackFactory;


} // namespace Utopia

#undef EXTENSIONFACTORY_templateparameterdecl

// generate specializations
#define BOOST_PP_ITERATION_LIMITS (0, EXTENSIONFACTORY_MAX_PARAMS - 1)
#define BOOST_PP_FILENAME_1 <utopia2/extensionfactory.h>
#include BOOST_PP_ITERATE()

#endif // Utopia_EXTENSIONFACTORY_H

#else // BOOST_PP_IS_ITERATING

#define n BOOST_PP_ITERATION()
#define EXTENSIONFACTORY_memberdecl(z, n, data) T ## n _p ## n;
#define EXTENSIONFACTORY_memberinit(z, n, data) _p ## n(p ## n)
#define EXTENSIONFACTORY_parameterdecl(z, n, data) const T ## n & p ## n

namespace Utopia
{
    // Specialisation patterns

    template< typename Extension, typename ExtensionDecl BOOST_PP_ENUM_TRAILING_PARAMS(n, typename T) >
    class ExtensionFactory< Extension, ExtensionDecl BOOST_PP_ENUM_TRAILING_PARAMS(n, T) >
        : public ExtensionFactoryBase< ExtensionDecl >
    {
    public:
        ExtensionFactory(BOOST_PP_ENUM(n, EXTENSIONFACTORY_parameterdecl, ~))
            : ExtensionFactoryBase< ExtensionDecl >()
              BOOST_PP_ENUM_TRAILING(n, EXTENSIONFACTORY_memberinit, ~)
        {}

        virtual ExtensionDecl * instantiate(bool singleton = false) const
        {
            ExtensionDecl * extension = 0;
            if (!singleton || !this->_singleton) {
                extension = static_cast< ExtensionDecl * >(new Extension(BOOST_PP_ENUM_PARAMS(n, _p)));
                if (singleton) {
                    this->_singleton.reset(extension);
                }
            } else {
                extension = this->_singleton.get();
            }
            return extension;
        }
    protected:
        BOOST_PP_REPEAT(n, EXTENSIONFACTORY_memberdecl, ~)
    };


    template< typename Extension, typename ExtensionDecl, typename Callback BOOST_PP_ENUM_TRAILING_PARAMS(n, typename T) >
    class ExtensionCallbackFactory< Extension, ExtensionDecl, Callback BOOST_PP_ENUM_TRAILING_PARAMS(n, T) >
        : public ExtensionFactoryBase< ExtensionDecl >
    {
    public:
        ExtensionCallbackFactory(const Callback & c BOOST_PP_ENUM_TRAILING(n, EXTENSIONFACTORY_parameterdecl, ~) )
            : ExtensionFactoryBase< ExtensionDecl >(), _c(c)
              BOOST_PP_ENUM_TRAILING(n, EXTENSIONFACTORY_memberinit, ~)
        {}

        virtual ExtensionDecl * instantiate(bool singleton = false) const
        {
            ExtensionDecl * extension = 0;
            if (!singleton || !this->_singleton) {
                extension = static_cast< ExtensionDecl * >(new Extension(BOOST_PP_ENUM_PARAMS(n, _p)));
                if (singleton) {
                    this->_singleton.reset(extension);
                }
            } else {
                extension = this->_singleton.get();
            }
            return extension;
        }
    protected:
        Callback _c;
        BOOST_PP_REPEAT(n, EXTENSIONFACTORY_memberdecl, ~)
    };
}

#undef EXTENSIONFACTORY_parameterdecl
#undef EXTENSIONFACTORY_memberinit
#undef EXTENSIONFACTORY_memberdecl
#undef n

#endif // BOOST_PP_IS_ITERATING
