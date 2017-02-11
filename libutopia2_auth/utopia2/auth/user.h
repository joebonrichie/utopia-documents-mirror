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

#ifndef KEND_USER_H
#define KEND_USER_H

#include <utopia2/auth/config.h>

#include <QDateTime>
#include <QObject>
#include <QPixmap>
#include <boost/shared_ptr.hpp>
#include <QString>

namespace Kend
{

    class Service;

    class UserPrivate;
    class User : public QObject
    {
        Q_OBJECT

    public:
        User(Service * service, const QString & id, QObject * parent = 0);
        ~User();

        // Meta information
        QDateTime created() const;
        QString errorCode() const;
        QString errorString() const;
        QString id() const;
        bool isAvatarModified() const;
        bool isAvatarReadOnly() const;
        bool isInfoModified() const;
        bool isModified() const;
        bool isNull() const;
        bool isReadOnly() const;
        QDateTime updated() const;

        // Information
        QPixmap avatar() const;
        QString get(const QString & key, const QString & defaultValue = QString()) const;
        QMap< QString, QString > info() const;
        void remove(const QString & key);
        void set(const QString & key, const QString & value);
        void setAvatar(const QPixmap & pixmap);

        // Aggregated information
        QString displayName() const;

        // Clone this user object
        User * clone(QObject * parent = 0) const;

        static QPixmap defaultAvatar();

    public slots:
        void commit(bool includeAvatar = true);
        void revert();
        void update();

    signals:
        // Has anything at all changed
        void avatarChanged();
        void avatarOverlayChanged();
        void commitCompleted(bool success);
        void commitFailed();
        void commitSucceeded();
        void displayNameChanged(const QString & displayName);
        void expired();
        void infoChanged();
        void infoOverlayChanged();
        void setup();

    protected:
        boost::shared_ptr< UserPrivate > d;
    };

} // namespace Kend

#endif // KEND_USER_H
