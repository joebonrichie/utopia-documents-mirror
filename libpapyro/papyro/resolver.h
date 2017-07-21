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

#ifndef ATHENAEUM_RESOLVER_H
#define ATHENAEUM_RESOLVER_H

#include <papyro/config.h>
#include <utopia2/extension.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Document.h>
#endif

#include <QStringList>
#include <QVariantList>

class QIODevice;

namespace Athenaeum
{

    class LibraryModel;

    class LIBPAPYRO_API Resolver
    {
    public:
        enum Purpose {
            Identify    = 0x0001,
            Dereference = 0x0002,
            Expand      = 0x0004,

            AllPurposes = 0xffff
        };
        Q_DECLARE_FLAGS(Purposes, Purpose)

        typedef Resolver API;
        virtual ~Resolver() {}

        virtual QVariantList resolve(const QVariantList & citations, Spine::DocumentHandle document = Spine::DocumentHandle()) = 0;

        virtual std::string title() = 0;
        virtual int weight() = 0;

        virtual Purposes purposes() = 0;

        virtual void cancel() {};

        std::string errorString() const { return _errorString; }
        void setErrorString(const std::string & errorString) { _errorString = errorString; }

    private:
        std::string _errorString;
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Athenaeum::Resolver::Purposes)

UTOPIA_DECLARE_EXTENSION_CLASS(LIBATHENAEUM, Athenaeum::Resolver)

#endif // ATHENAEUM_RESOLVER_H
