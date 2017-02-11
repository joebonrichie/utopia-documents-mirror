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

#ifndef UTOPIA_EXTENSIONLIBRARY_H
#define UTOPIA_EXTENSIONLIBRARY_H

#include <utopia2/config.h>

#include <boost/scoped_ptr.hpp>
#include <set>
#include <string>

#include <QDir>

#define UTOPIA_EXTENSION_LIBRARY_VERSION "Utopia::ExtensionLibrary 1.0"

namespace Utopia
{

    class Library;

    /**
     *  \class ExtensionLibrary
     */
    class LIBUTOPIA_API ExtensionLibrary {

    public:
        ~ExtensionLibrary();

        // Filename
        QString filename() const;

        // API
        typedef const char * (*apiVersionFn)();
        typedef const char * (*descriptionFn)();
        typedef void (*registerExtensionsFn)();

        const QString & description() const;

        // Static Library loading functions
        static ExtensionLibrary * load(const QString & filename_);
        static QSet< ExtensionLibrary * > loadDirectory(const QDir & directory_, bool recursive_ = false);
        static ExtensionLibrary * wrap(Library * library);

    private:
        // Construct Library object
        ExtensionLibrary(Library * library, const QString & description);

        QString _description;

        boost::scoped_ptr< Library > _library;

    }; // class ExtensionLibrary

} // namespace Utopia

#endif // Utopia_EXTENSIONLIBRARY_H
