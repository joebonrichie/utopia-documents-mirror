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

#include <utopia2/auth/qt/userlabel_p.h>
#include <utopia2/auth/qt/userlabel.h>
#include <utopia2/auth/service.h>
#include <utopia2/auth/user.h>

#include <QPainter>

#include <QDebug>

namespace Kend
{

    UserLabelPrivate::UserLabelPrivate(User * user, UserLabel::UserData type, UserLabel * label)
        : QObject(label), label(label), type(type), hasRoundedCorners(false), clickable(false)
    {
        reset(user);
    }

    void UserLabelPrivate::onUserChanged()
    {
        if (user) {
            if (type == UserLabel::Avatar) {
                label->setPixmap(user->avatar());
            } else if (type == UserLabel::DisplayName) {
                if (user->isNull()) {
                    label->setText("Fetching...");
                } else {
                    QString displayName = QString("%1 %2 %3").arg(user->get("title"),
                                                                  user->get("forename"),
                                                                  user->get("surname"));
                    displayName = displayName.trimmed().replace(QRegExp("\\s+"), " ");

                    if (displayName.isEmpty()) {
                        displayName = "Unnamed user";
                    }

                    label->setText(displayName);
                }
            }
        } else {
            if (type == UserLabel::Avatar) {
                label->setPixmap(User::defaultAvatar());
            } else if (type == UserLabel::DisplayName) {
                label->setText("Anonymous user");
            }
        }
    }

    void UserLabelPrivate::reset(User * newUser)
    {
        user.reset(newUser);

        label->QLabel::clear();

        if (newUser) {
            connect(newUser, SIGNAL(expired()), this, SLOT(reset()));
            connect(newUser, SIGNAL(setup()), this, SLOT(onUserChanged()));
            connect(newUser, SIGNAL(infoChanged()), this, SLOT(onUserChanged()));
            connect(newUser, SIGNAL(infoOverlayChanged()), this, SLOT(onUserChanged()));
            connect(newUser, SIGNAL(avatarChanged()), this, SLOT(onUserChanged()));
            connect(newUser, SIGNAL(avatarOverlayChanged()), this, SLOT(onUserChanged()));
        }

        onUserChanged();
    }






    UserLabel::UserLabel(Service * service, const QString & id, UserData type, QWidget * parent)
        : QLabel(parent), d(new UserLabelPrivate(service->user(id), type, this))
    {}

    UserLabel::UserLabel(User * user, UserData type, QWidget * parent)
        : QLabel(parent), d(new UserLabelPrivate(user, type, this))
    {}

    UserLabel::UserLabel(UserData type, QWidget * parent)
        : QLabel(parent), d(new UserLabelPrivate(0, type, this))
    {}

    void UserLabel::clear()
    {
        d->reset();
    }

    bool UserLabel::hasRoundedCorners() const
    {
        return d->hasRoundedCorners;
    }

    bool UserLabel::isClickable() const
    {
        return d->clickable;
    }

    void UserLabel::mouseReleaseEvent(QMouseEvent * event)
    {
        if (d->clickable) {
            emit clicked();
        }
    }

    void UserLabel::paintEvent(QPaintEvent * event)
    {
        if (pixmap()) {
            QSize scaled(pixmap()->size());
            scaled.scale(size(), Qt::KeepAspectRatio);
            QPoint origin((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
            QPixmap cropped(scaled);
            cropped.fill(QColor(0, 0, 0, 0));

            QPainter compPainter(&cropped);
            compPainter.setRenderHints(QPainter::SmoothPixmapTransform, true);
            compPainter.setRenderHints(QPainter::Antialiasing, true);
            compPainter.setBrush(Qt::white);
            compPainter.setPen(Qt::NoPen);
            if (d->hasRoundedCorners) {
                compPainter.drawRoundedRect(cropped.rect(), 5, 5);
            } else {
                compPainter.drawRect(cropped.rect());
            }
            compPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            compPainter.drawPixmap(cropped.rect(), *pixmap());

            QPainter painter(this);
            painter.drawPixmap(origin, cropped);
        } else {
            QLabel::paintEvent(event);
        }
    }

    void UserLabel::setClickable(bool clickable)
    {
        d->clickable = clickable;
        setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }

    void UserLabel::setRoundedCorners(bool rounded)
    {
        d->hasRoundedCorners = rounded;
        update();
    }

    void UserLabel::setUser(User * user)
    {
        d->reset(user);
    }

    void UserLabel::setUser(Service * service, const QString & id)
    {
        setUser(service ? service->user(id) : 0);
    }

} // namespace Kend
