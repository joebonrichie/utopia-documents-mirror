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

#ifndef KEND_USER_P_H
#define KEND_USER_P_H

#include <utopia2/auth/config.h>
#include <utopia2/networkaccessmanager.h>

#include <boost/shared_ptr.hpp>

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QUrl>

namespace Kend
{

    class Service;

    class User;
    class UserPrivate : public QObject, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        enum Task {
            Idle                = 0x0,
            UpdateInfoTask      = 0x1,
            UpdateAvatarTask    = 0x2,
            CommitInfoTask      = 0x4
        };
        Q_ENUMS(Task)
        Q_DECLARE_FLAGS(Tasks, Task)

        static boost::shared_ptr< UserPrivate > create(Service * service, const QString & id);
        virtual ~UserPrivate() {}

        // Public object / state
        QPointer< Service > service;
        Tasks running;
        bool isNull;
        int redirects;
        bool failed;

        // Metadata
        QUrl avatar_uri;
        QUrl avatar_edit_uri;
        QDateTime created;
        QUrl edit_uri;
        QString errorCode;
        QString errorString;
        QString id;
        QDateTime updated;

        // Core data
        QMap< QString, QString > info;
        QMap< QString, QString > overlay;
        QSet< QString > mask;
        QString displayName;
        QPixmap avatar;
        QPixmap avatarOverlay;

        // Modify this user's information
        bool isModified(bool includeAvatar = true) const;
        void removeValue(const QString & key);
        void resetAvatar();
        void resetInfo();
        void setValue(const QString & key, const QString & value);

        // Computed info
        QMap< QString, QString > computeInfo() const;
        QString computeValue(const QString & key, const QString & defaultValue = QString()) const;

        // Setters
        void setAvatar(const QPixmap & newAvatar);
        void setInfo(const QMap< QString, QString > & newData);
        void setDisplayName(QString newDisplayName = QString());

    public slots:
        // Fetching
        void fetchInfo();
        void fetchAvatar();
        void putInfo(bool includeAvatar);

    signals:
        void avatarChanged();
        void avatarOverlayChanged();
        void commitCompleted(bool success);
        void commitFailed();
        void commitSucceeded();
        void infoChanged();
        void infoOverlayChanged();
        void displayNameChanged(const QString & displayName);
        void setup();

    protected slots:
        void onFinished();

    protected:
        UserPrivate(Service * service, const QString & id);

        friend class User;
    };


} // namespace Kend

#endif // KEND_USER_P_H
