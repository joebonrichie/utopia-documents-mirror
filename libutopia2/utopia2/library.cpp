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

#include <utopia2/library.h>

#include <boost/scoped_array.hpp>

#include <iostream>
#include <stdio.h>

#include <QDir>
#include <QDebug>

#ifdef __WIN32__
#include <windows.h>
#else
#include <dlfcn.h>
#endif


namespace Utopia
{

    // Anonymous namespace
    namespace
    {

        /**
         *  \brief Load dynamically loadable module.
         *  \param path_ Full path to Library file.
         *
         *  Returns zero on error.
         */

        inline void * loadLibrary(const QString & path_)
        {
            qDebug() << "Loading Library" << path_;

#ifdef _WIN32
            /*clear the last error*/
            SetLastError(0);
            return (void *) LoadLibraryW((LPWSTR) path_.utf16());
#else
            return ::dlopen(path_.toUtf8().constData(), RTLD_LAZY | RTLD_GLOBAL);
#endif
        }

        /**
         *  \brief Unload dynamically loaded module.
         *  \param handle_ Handle to module.
         *
         *  Returns zero on error.
         */
        inline bool unloadLibrary(void * handle_)
        {
#ifdef _WIN32
            return FreeLibrary((HMODULE) handle_);
#else
            return (::dlclose(handle_) == 0);
#endif
        }

        /**
         *  \brief Load symbol from dynamically loaded Library.
         *  \param handle_ Handle to loaded Library.
         *  \param name_ Symbol name to load.
         *
         *  Returns zero on error.
         */
        inline void* librarySymbol(void * handle_, const QString & name_)
        {
#ifdef _WIN32
            return (void*) GetProcAddress((HMODULE) handle_, name_.toUtf8().constData());
#else
            return ::dlsym(handle_, name_.toUtf8().constData());
#endif
        }

        /** Check error string for Library functions. */
        inline QString libraryError(void)
        {
            // Return error string
#ifdef _WIN32
            int len, errorCode;
            static WCHAR errstr[256];

            if ((errorCode = GetLastError()) == 0)
            {
                return "No error string available";
            }

            /* format the error string */
            len = swprintf(errstr, L"Error <%d>: ", errorCode);
            len += FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
                (LPWSTR) errstr + len,
                256 - len,
                NULL
                );

            /*replace \r\n*/
            if (len > 1 && errstr[len-2] == L'\r' && errstr[len-1] == L'\n') {
                if (len > 2 && errstr[len-3] == L'.')
                    len--;
            }

            SetLastError(0);
            return QString::fromUtf16((const unsigned short *) errstr);
#else
            return QString::fromUtf8(::dlerror());
#endif
        }

    }




    Library::Library(const QString & filename_, void * handle)
        : _filename(filename_), _handle(handle)

    {}

    Library::~Library()
    {
        if (!unloadLibrary(_handle))
        {
            qDebug() << "Unable to unload library" << _filename;
            qDebug() << " -" << libraryError();
        }

    }

    const QString & Library::filename() const
    {
        return _filename;
    }

    QString Library::lastError()
    {
        return libraryError();
    }

    Library * Library::load(const QString & path_)
    {
        void * handle = loadLibrary(path_);
        return handle ? new Library(path_, handle) : 0;
    }

    QSet< Library * > Library::loadDirectory(const QDir & directory, bool recursive_)
    {
        QSet< Library * > libraries;

        if (!directory.exists()) {
            qDebug() << "Library::loadDirectory(): Path does not exist:" << directory.absolutePath();
        } else {
            foreach (QFileInfo fileInfo, directory.entryInfoList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable)) {
                if (fileInfo.isDir()) {
                    if (recursive_) {
                        libraries.unite(loadDirectory(fileInfo.dir()));
                    }
                } else if (fileInfo.isFile()) {
                    QString path = fileInfo.canonicalFilePath();
                    QString filename = fileInfo.fileName();

                    // Only try to load certain files (no _*, *.py*)
                    if (!filename.isEmpty() && filename[0] != '_' && fileInfo.suffix() != "py") {
                        if (Library * library = load(path)) {
                            //std::cerr << "Library::loadDirectory(): Successfully loaded library " << path << std::endl;
                            libraries.insert(library);
                        } else {
                            qDebug() << "Library::loadDirectory(): Unable to load library" << path;
                            qDebug() << "                        :" << libraryError();
                        }
                    }
                }
            }
        }

        return libraries;
    }

    void * Library::symbol(const QString & symbol_) const
    {
        return librarySymbol(_handle, symbol_);;
    }

} // namespace utopia
