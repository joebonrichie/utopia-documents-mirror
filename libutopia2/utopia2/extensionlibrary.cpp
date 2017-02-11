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

#include <utopia2/config.h>
#include <utopia2/extensionlibrary.h>

#include <utopia2/library.h>
#include <cstring>
#include <stdio.h>

#include <QDebug>

namespace Utopia
{

    namespace
    {

        class ExtensionLibraryRegistry
        {
        public:
            ~ExtensionLibraryRegistry()
            {
                // Delete extension libraries
                QSet< ExtensionLibrary * > doomed(extensionLibraries);
                foreach (ExtensionLibrary * extensionLibrary, doomed) {
                    delete extensionLibrary;
                }
            }

            static void registerExtensionLibrary(ExtensionLibrary * extensionLibrary)
            {
                instance().extensionLibraries.insert(extensionLibrary);
            }

            static void unregisterExtensionLibrary(ExtensionLibrary * extensionLibrary)
            {
                instance().extensionLibraries.remove(extensionLibrary);
            }

        protected:
            static ExtensionLibraryRegistry & instance()
            {
                static ExtensionLibraryRegistry registry;
                return registry;
            }

            QSet< ExtensionLibrary * > extensionLibraries;
        };

    }




    ExtensionLibrary::ExtensionLibrary(Library * library, const QString & description)
        : _description(description), _library(library)
    {
        ExtensionLibraryRegistry::registerExtensionLibrary(this);
    }

    ExtensionLibrary::~ExtensionLibrary()
    {
        ExtensionLibraryRegistry::unregisterExtensionLibrary(this);
    }

    const QString & ExtensionLibrary::description() const
    {
        return _description;
    }

    QString ExtensionLibrary::filename() const
    {
        // FIXME: should this be a const char * and return NULL if no Library?
        return _library.get() ? _library->filename() : QString();
    }

    ExtensionLibrary * ExtensionLibrary::load(const QString & path_)
    {
        if (Library * library = Library::load(path_)) {
            if (ExtensionLibrary * extensionLibrary = wrap(library)) {
                return extensionLibrary;
            } else {
                delete library;
            }
        }
        return 0;
    }

    QSet< ExtensionLibrary * > ExtensionLibrary::loadDirectory(const QDir & directory_, bool recursive_)
    {
        QSet< ExtensionLibrary * > extensionLibraries;
        QSet< Library * > libraries(Library::loadDirectory(directory_, recursive_));
        foreach (Library * library, libraries) {
            if (ExtensionLibrary * extensionLibrary = wrap(library)) {
                extensionLibraries.insert(extensionLibrary);
            } else {
                // Not a valid Utopia extension library
                delete library;
            }
        }
        return extensionLibraries;
    }

    ExtensionLibrary * ExtensionLibrary::wrap(Library * library)
    {
        if (library) {
            // Only conforming libraries can be loaded as an ExtensionLibrary
            apiVersionFn apiVersion = (apiVersionFn) (unsigned long long) library->symbol("utopia_apiVersion");
            descriptionFn description = (descriptionFn) (unsigned long long) library->symbol("utopia_description");
            registerExtensionsFn registerExtensions = (registerExtensionsFn) (unsigned long long) library->symbol("utopia_registerExtensions");

            // Silently fail if API is incorrect
            if (registerExtensions && description && apiVersion && std::strcmp(apiVersion(), UTOPIA_EXTENSION_LIBRARY_VERSION) == 0) {
                qDebug() << "  " << description();
                ExtensionLibrary * extensionLibrary = new ExtensionLibrary(library, description());
                registerExtensions();
                return extensionLibrary;
            } else if (apiVersion) {
                qDebug() << "Wrong Library Version:" << QString("[%1]").arg(apiVersion()) << library->filename();
            } else {
                qDebug() << "Wrong Library Version:" << library->filename();
            }
        }

        return 0;
    }

} /* namespace Utopia */
