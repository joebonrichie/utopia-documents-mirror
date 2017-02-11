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

#ifndef UTOPIA_GLOBAL_H
#define UTOPIA_GLOBAL_H

#include <utopia2/config.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QObject>
#include <QVariantMap>

namespace Utopia
{

    class PACProxyFactory;

    class LIBUTOPIA_API SplashHelper : public QObject
    {
        Q_OBJECT

    public:
        SplashHelper(QObject * splash)
            : QObject()
        {
            connect(this, SIGNAL(messageChanged(QString)), splash, SLOT(changeMessage(QString)));
        }

        void changeMessage(const QString & message)
        {
            Q_EMIT messageChanged(message);
        }

    Q_SIGNALS:
        void messageChanged(const QString & message);
    };

    LIBUTOPIA_EXPORT void init(QObject* progressIndicator_ = 0);

    LIBUTOPIA_EXPORT int versionMajor();
    LIBUTOPIA_EXPORT int versionMinor();
    LIBUTOPIA_EXPORT int versionPatch();

    LIBUTOPIA_EXPORT QString versionString(bool forceDebug = false);
    LIBUTOPIA_EXPORT QString revisionHash();
    LIBUTOPIA_EXPORT QString revisionBranch();

    LIBUTOPIA_EXPORT PACProxyFactory * globalProxyFactory();

    const QVariantMap & defaults();

    LIBUTOPIA_EXPORT QString current_path();
    LIBUTOPIA_EXPORT QString executable_path();
    LIBUTOPIA_EXPORT QString plugin_path();
    LIBUTOPIA_EXPORT QString config_path();
    LIBUTOPIA_EXPORT QString private_library_path();
    LIBUTOPIA_EXPORT QString resource_path();

    typedef enum {
        ProfileRoot,
        ProfilePlugins,
        ProfileData,
        ProfileLogs
    } ProfilePathPart;
    LIBUTOPIA_EXPORT QString profile_path(ProfilePathPart part = ProfileRoot);

} /* namespace Utopia */




template <typename T> static inline T *uGetPtrHelper(T *ptr) { return ptr; }
template <typename Wrapper> static inline typename Wrapper::pointer uGetPtrHelper(const Wrapper &p) { return p.get(); }

#define U_DECLARE_PRIVATE(Class)                                        \
    inline Class##Private* __private_func() { return reinterpret_cast<Class##Private *>(uGetPtrHelper(this->__private)); } \
    inline const Class##Private* __private_func() const { return reinterpret_cast<const Class##Private *>(uGetPtrHelper(this->__private)); } \
    friend class Class##Private;
#define U_DEFINE_PRIVATE(Class)                 \
    Class##Private * __private;

#define U_DECLARE_PUBLIC(Class)                                         \
    inline Class* __public_func() { return static_cast<Class *>(this->__public); } \
    inline const Class* __public_func() const { return static_cast<const Class *>(this->__public); } \
    friend class Class;
#define U_DEFINE_PUBLIC(Class)                  \
    Class * __public;

#define U_GET_PRIVATE(Object) Object->__private_func()
#define U_GET_PUBLIC(Object) Object->__public_func()

#define U_D(Class) Class##Private * const d = U_GET_PRIVATE(this)
#define U_U(Class) Class * const u = U_GET_PUBLIC(this)

#endif // Utopia_GLOBAL_H
