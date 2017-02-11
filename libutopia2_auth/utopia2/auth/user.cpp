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

#include <utopia2/auth/user_p.h>
#include <utopia2/auth/user.h>
#include <utopia2/auth/service.h>

#include <boost/weak_ptr.hpp>

#include <QBuffer>
#include <QDomDocument>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QRegExp>
#include <QTimer>
#include <QUrlQuery>

#include <QDebug>

Q_DECLARE_METATYPE(Kend::UserPrivate::Task);

namespace Kend
{

    UserPrivate::UserPrivate(Service * service, const QString & id)
        : QObject(), service(service), running(Idle), isNull(true), redirects(0), failed(false), id(id), displayName("Fetching...")
    {
        if (service) {
            // Generate the fetching URL for a user's details. We ask the users collection, passing
            // in the user URI as a query item:
            //    {AUTH_URI}/users?user={USER_URI}
            QUrl authUrl(service->resourceUrl(Service::AuthenticationResource));
            if (authUrl.isValid()) {
                authUrl.setPath(authUrl.path() + "/users");
                QUrlQuery query(authUrl.query());
                query.addQueryItem("user", id);
                authUrl.setQuery(query);
                edit_uri = authUrl;
            }

            // Schedule a full update on the next event
            QTimer::singleShot(0, this, SLOT(fetchInfo()));
        }
    }

    QMap< QString, QString > UserPrivate::computeInfo() const
    {
        // Take the original data...
        QMap< QString, QString > computed(info);
        // ...overlay with new data...
        QMapIterator< QString, QString > incoming(overlay);
        while (incoming.hasNext()) {
            incoming.next();
            computed[incoming.key()] = incoming.value();
        }
        // ...then remove any deleted fields.
        QSetIterator< QString > doomed(mask);
        while (doomed.hasNext()) {
            computed.remove(doomed.next());
        }
        return computed;
    }

    QString UserPrivate::computeValue(const QString & key, const QString & defaultValue) const
    {
        QString computed(defaultValue);
        if (!mask.contains(key)) {
            computed = overlay.value(key);
            if (computed.isNull()) {
                computed = info.value(key, defaultValue);
            }
        }
        return computed;
    }

    boost::shared_ptr< UserPrivate > UserPrivate::create(Service * service, const QString & id)
    {
        // Get new private object, or share an old one if the id matches
        static QMap< QPair< Service *, QString >, boost::weak_ptr< UserPrivate > > knownUsers;
        boost::shared_ptr< UserPrivate > found(knownUsers.value(qMakePair(service, id)).lock());
        if (!found) {
            found = boost::shared_ptr< UserPrivate >(new UserPrivate(service, id));
            knownUsers[qMakePair(service, id)] = found;
        }

        // Remove stale pointers
        QMutableMapIterator< QPair< Service *, QString >, boost::weak_ptr< UserPrivate > > deathRow(knownUsers);
        while (deathRow.hasNext()) {
            deathRow.next();
            if (deathRow.value().expired()) {
                deathRow.remove();
            }
        }

        return found;
    }

    void UserPrivate::fetchInfo()
    {
        if (service && (isNull || edit_uri.isValid()) && !(running & UpdateInfoTask)) {
            QNetworkReply * reply = service->get(QNetworkRequest(edit_uri));
            reply->setProperty("task", QVariant::fromValue< Task >(UpdateInfoTask));
            connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
            running |= UpdateInfoTask;
        }
    }

    void UserPrivate::fetchAvatar()
    {
        if (service && avatar_uri.isValid() && !(running & UpdateAvatarTask)) {
            QNetworkReply * reply = service->get(QNetworkRequest(avatar_uri));
            reply->setProperty("task", QVariant::fromValue< Task >(UpdateAvatarTask));
            connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
            running |= UpdateAvatarTask;
        }
    }

    bool UserPrivate::isModified(bool includeAvatar) const
    {
        return (includeAvatar && !avatarOverlay.isNull()) || computeInfo() != info;
    }

    void UserPrivate::onFinished()
    {
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
        reply->deleteLater();

        Task task = reply->property("task").value< Task >();

        // If this is a redirect, follow it transparently, but only to a maximum of (arbitrarily)
        // four redirections
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectedUrl.isValid()) {
            if (redirectedUrl.isRelative()) {
                QString redirectedAuthority = redirectedUrl.authority();
                redirectedUrl = reply->url().resolved(redirectedUrl);
                if (!redirectedAuthority.isEmpty()) {
                    redirectedUrl.setAuthority(redirectedAuthority);
                }
            }
            if (redirects++ < 4 && reply->operation() == QNetworkAccessManager::GetOperation) {
                QNetworkRequest request = reply->request();
                request.setUrl(redirectedUrl);
                service->get(request)->setProperty("task", QVariant::fromValue< Task >(task));
                return;
            } else {
                // FIXME handle error
            }
        }
        // Reset redirect count
        redirects = 0;

        switch (reply->error()) {
        case QNetworkReply::NoError:
            errorCode = QString();
            errorString = QString();
            switch (task) {
            case CommitInfoTask:
                // Remove overlay and mask
                // FIXME only do this on definite success
                resetInfo();
            case UpdateInfoTask:
                {
                    QDomDocument doc;
                    doc.setContent(reply);
                    //QByteArray bytes(reply->readAll());
                    //qDebug() << "<<<<<<<<" << bytes;
                    //doc.setContent(bytes);

                    QDomElement userElem = doc.documentElement();
                    if (userElem.tagName() == "user" && userElem.attribute("version") == "0.3") {
                        edit_uri = QUrl(userElem.attribute("edit"));
                        created = QDateTime::fromString(userElem.attribute("created"), Qt::ISODate);
                        updated = QDateTime::fromString(userElem.attribute("updated"), Qt::ISODate);
                        avatar_uri = QUrl(userElem.attribute("avatar"));
                        avatar_edit_uri = QUrl(userElem.attribute("avatar-edit"));
                        if (isNull) {
                            isNull = false;
                            emit setup();
                        }
                        QMap< QString, QString > info;
                        QDomElement infoElem = userElem.firstChildElement("info");
                        while (!infoElem.isNull()) {
                            QString key(infoElem.attribute("name"));
                            QString value(infoElem.text());
                            if (!key.isEmpty() && !value.isEmpty()) {
                                info[key] = value;
                            }
                            infoElem = infoElem.nextSiblingElement("info");
                        }
                        setInfo(info);
                        fetchAvatar();
                    }
                }
                break;
            case UpdateAvatarTask:
                {
                    QImage image(QImage::fromData(reply->readAll()));
                    QPixmap pixmap(QPixmap::fromImage(image));
                    resetAvatar();
                    setAvatar(pixmap);
                }
                break;
            default:
                break;
            }
            break;
        default:
            switch (task) {
            case CommitInfoTask:
                {
                    // FIXME deal with error
                    failed = true;
                    QByteArray response(reply->readAll());
                    //qDebug() << "------" << reply->error();
                    //qDebug() << response;
                    QVariantMap error = QJsonDocument::fromJson(response).toVariant().toMap();
                    errorCode = error.value("error_code").toString();
                    errorString = error.value("error_string").toString();
                }
                break;
            case UpdateInfoTask:
                break;
            case UpdateAvatarTask:
                break;
            default:
                break;
            }
            break;
        }

        running &= ~task;

        // Alert on success or failure
        if (running == Idle) {
            emit commitCompleted(!failed);
            if (failed) {
                emit commitFailed();
            } else {
                emit commitSucceeded();
            }
        }
    }

    void UserPrivate::putInfo(bool includeAvatar)
    {
        if (service && edit_uri.isValid() && !(running & CommitInfoTask)) {
            // Check if nothing has changed, in which case, don't do anything!
            if (isModified(includeAvatar)) {
                QMap< QString, QString > computedInfo(computeInfo());
                // Templates for generating XML (by hand at the moment)
                static const QString user_tpl(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<user xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                         " xmlns=\"http://utopia.cs.manchester.ac.uk/authd\""
                         " version=\"0.3\""
                         " service=\"%1\""
                         " xsi:schemaLocation=\"http://utopia.cs.manchester.ac.uk/authd https://utopia.cs.manchester.ac.uk/authd/0.3/xsd/user\">"
                    "%2"
                    "</user>");
                static const QString info_tpl("<info name=\"%1\">%2</info>");

                // Get computed info
                QMapIterator< QString, QString > computed(computedInfo);
                QString infoStr;
                while (computed.hasNext()) {
                    computed.next();
                    QString key(computed.key());
                    QString value(computed.value());
                    infoStr += info_tpl.arg(key.replace("\"", "&quot;"), value.replace("<", "&lt;"));
                }

                // Add avatar if needed
                if (includeAvatar && avatar_edit_uri.isValid() && !avatarOverlay.isNull()) {
                    // Encode base 64 PNG data
                    QBuffer data;
                    data.open(QIODevice::WriteOnly);
                    avatarOverlay.toImage().save(&data, "PNG");
                    data.close();
                    infoStr += info_tpl.arg("avatar", QString::fromUtf8(data.data().toBase64()));
                }

                QString userStr = user_tpl.arg(service->authenticationMethod(), infoStr);

                //qDebug() << ">>>>>>>>" << userStr;
                QNetworkReply * reply = service->put(QNetworkRequest(edit_uri), userStr.toUtf8(), "application/x-authd+xml;version=0.3;type=user");
                reply->setProperty("task", QVariant::fromValue< Task >(CommitInfoTask));
                connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
                running |= CommitInfoTask;
            }
        }
    }

    void UserPrivate::removeValue(const QString & key)
    {
        overlay.remove(key);
        mask.insert(key);
        emit infoOverlayChanged();
    }

    void UserPrivate::resetAvatar()
    {
        avatarOverlay = QPixmap();
        emit avatarOverlayChanged();
    }

    void UserPrivate::resetInfo()
    {
        overlay.clear();
        mask.clear();
        emit infoOverlayChanged();
    }

    void UserPrivate::setValue(const QString & key, const QString & value)
    {
        mask.remove(key);
        overlay[key] = value;
        emit infoOverlayChanged();
    }

    void UserPrivate::setAvatar(const QPixmap & newAvatar)
    {
        avatar = newAvatar;
        emit avatarChanged();
    }

    void UserPrivate::setInfo(const QMap< QString, QString > & newData)
    {
        info = newData;
        emit infoChanged();
        setDisplayName();
    }

    void UserPrivate::setDisplayName(QString newDisplayName)
    {
        if (newDisplayName.isNull()) {
            newDisplayName = QString("%1 %2 %3").arg(computeValue("title", ""),
                                                     computeValue("forename", ""),
                                                     computeValue("surname", "")).trimmed().replace(QRegExp("\\s+"), " ");
        }
        if (newDisplayName.isEmpty()) {
            newDisplayName = "Unnamed user";
        }
        if (displayName != newDisplayName) {
            displayName = newDisplayName;
            if (!displayName.isEmpty()) {
                emit displayNameChanged(displayName);
            }
        }
    }




    User::User(Service * service, const QString & id, QObject * parent)
        : QObject(parent), d(UserPrivate::create(service, id))
    {
        connect(d.get(), SIGNAL(avatarChanged()), this, SIGNAL(avatarChanged()));
        connect(d.get(), SIGNAL(avatarOverlayChanged()), this, SIGNAL(avatarOverlayChanged()));
        connect(d.get(), SIGNAL(commitCompleted(bool)), this, SIGNAL(commitCompleted(bool)));
        connect(d.get(), SIGNAL(commitFailed()), this, SIGNAL(commitFailed()));
        connect(d.get(), SIGNAL(commitSucceeded()), this, SIGNAL(commitSucceeded()));
        connect(d.get(), SIGNAL(displayNameChanged(const QString &)), this, SIGNAL(displayNameChanged(const QString &)));
        connect(d.get(), SIGNAL(infoChanged()), this, SIGNAL(infoChanged()));
        connect(d.get(), SIGNAL(infoOverlayChanged()), this, SIGNAL(infoOverlayChanged()));
        connect(d.get(), SIGNAL(setup()), this, SIGNAL(setup()));

        if (service) {
            // Keep track of object deletion
            connect(service, SIGNAL(destroyed()), this, SIGNAL(expired()));
        }
    }

    User::~User()
    {}

    QPixmap User::avatar() const
    {
        if (!d->avatarOverlay.isNull()) {
            return d->avatarOverlay;
        } else if (!d->avatar.isNull()) {
            return d->avatar;
        } else {
            return defaultAvatar();
        }
    }

    User * User::clone(QObject * parent) const
    {
        return new User(d->service, d->id, parent);
    }

    void User::commit(bool includeAvatar)
    {
        if (!isNull()) {
            d->failed = false;
            d->putInfo(includeAvatar);
        }
    }

    QDateTime User::created() const
    {
        return d->created;
    }

    QPixmap User::defaultAvatar()
    {
        static const QPixmap pixmap(":/icons/default-avatar.png");
        return pixmap;
    }

    QString User::displayName() const
    {
        return d->displayName;
    }

    QString User::errorCode() const
    {
        return d->errorCode;
    }

    QString User::errorString() const
    {
        return d->errorString;
    }

    QString User::get(const QString & key, const QString & defaultValue) const
    {
        return key == "avatar" ? QString() : d->computeValue(key, defaultValue);
    }

    QString User::id() const
    {
        return d->id;
    }

    QMap< QString, QString > User::info() const
    {
        return d->computeInfo();
    }

    bool User::isAvatarModified() const
    {
        return !d->avatarOverlay.isNull();
    }

    bool User::isAvatarReadOnly() const
    {
        return isNull() || !d->avatar_edit_uri.isValid();
    }

    bool User::isInfoModified() const
    {
        return d->isModified(false);
    }

    bool User::isModified() const
    {
        return d->isModified();
    }

    bool User::isNull() const
    {
        return !d->service || d->isNull;
    }

    bool User::isReadOnly() const
    {
        return isNull() || !d->edit_uri.isValid();
    }

    void User::remove(const QString & key)
    {
        d->removeValue(key);
    }

    void User::revert()
    {
        d->resetAvatar();
        d->resetInfo();
    }

    void User::set(const QString & key, const QString & value)
    {
        if (key != "avatar") {
            d->setValue(key, value);
        }
    }

    void User::setAvatar(const QPixmap & pixmap)
    {
        d->avatarOverlay = pixmap;
        emit d->avatarOverlayChanged();
    }

    void User::update()
    {
        d->fetchInfo();
    }

    QDateTime User::updated() const
    {
        return d->updated;
    }

} // namespace Kend
