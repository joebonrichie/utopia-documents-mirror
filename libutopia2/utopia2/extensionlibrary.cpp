/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

    ExtensionLibrary::ExtensionLibrary(Library * library, const QString & description)
        : _library(library), _description(description)
    {}

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
        return sanitise(Library::load(path_));
    }

    QSet< ExtensionLibrary * > ExtensionLibrary::loadDirectory(const QDir & directory_, bool recursive_)
    {
        return sanitise(Library::loadDirectory(directory_, recursive_));
    }

    ExtensionLibrary * ExtensionLibrary::sanitise(Library * library)
    {
        if (library)
        {
            // Only conforming libraries can be loaded as an ExtensionLibrary
            apiVersionFn apiVersion = (apiVersionFn) (long) library->symbol("utopia_apiVersion");
            descriptionFn description = (descriptionFn) (long) library->symbol("utopia_description");
            registerExtensionsFn registerExtensions = (registerExtensionsFn) (long) library->symbol("utopia_registerExtensions");

            // Silently fail if API is incorrect
            if (registerExtensions && description && apiVersion && std::strcmp(apiVersion(), UTOPIA_EXTENSION_LIBRARY_VERSION) == 0)
            {
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

    QSet< ExtensionLibrary * > ExtensionLibrary::sanitise(const QSet< Library * > & libraries)
    {
        QSet< ExtensionLibrary * > extensionLibraries;
        foreach (Library * library, libraries) {
            if (ExtensionLibrary * extensionLibrary = sanitise(library)) {
                extensionLibraries.insert(extensionLibrary);
            }
        }
        return extensionLibraries;
    }

} /* namespace Utopia */
