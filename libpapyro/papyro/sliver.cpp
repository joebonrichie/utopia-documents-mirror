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

#include <papyro/sliver_p.h>
#include <papyro/sliver.h>

#include <utopia2/qt/slidelayout.h>

#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>

#include <QDebug>

namespace Papyro
{

    SliverPrivate::SliverPrivate(Sliver * sliver)
        : QObject(sliver), sliver(sliver), taskColor(220, 0, 0)
    {
        // Connections
        connect(this, SIGNAL(titleChanged(const QString &)), sliver, SIGNAL(titleChanged(const QString &)));

        setTitle("");
    }

    bool SliverPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        if (obj == taskWidget) {
            if (!title.isEmpty() && event->type() == QEvent::Paint) {
                QRect rect = QRect(0, 0, taskWidget->height(), taskWidget->width()).adjusted(4, 0, -8, 0);
                QPainter painter(taskWidget);
                painter.rotate(270);
                painter.translate(-taskWidget->height(), 0);
                painter.setPen(Qt::white);
                painter.setBrush(Qt::NoBrush);
                painter.setOpacity(0.8);
                QFont font(painter.font());
                font.setPointSize(18);
                font.setItalic(true);
                QFontMetrics fontMetrics(font);
                QString elided(fontMetrics.elidedText(title, Qt::ElideRight, rect.width()));
                painter.setFont(font);
                painter.setRenderHint(QPainter::TextAntialiasing, true);
                painter.drawText(rect, (Qt::AlignVCenter | Qt::AlignRight), elided);
                return true;
            } else {
                return false;
            }
        }
        return QObject::eventFilter(obj, event);
    }

    void SliverPrivate::init()
    {
        //sliver->setObjectName("sliver");
        //sliver->setAutoFillBackground(true);

        taskWidget = new QWidget;
        taskWidget->installEventFilter(this);

        //slideLayout = new Utopia::SlideLayout(Utopia::SlideLayout::StackLeft);
        //slideLayout->addWidget(taskWidget);
        //slideLayout->setSlideDuration(150);
        //slideLayout->setUpdateInterval(20);
        //connect(slideLayout, SIGNAL(animated()), sliver, SLOT(update()));

        QVBoxLayout * layout = new QVBoxLayout(sliver);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        //layout->addLayout(slideLayout, 100);
    }

    void SliverPrivate::setTaskColor(const QColor & newColor)
    {
        if (taskColor != newColor) {
            taskColor = newColor;
            sliver->update();
        }
    }

    void SliverPrivate::setTitle(const QString & newTitle)
    {
        if (title != newTitle) {
            //bool entering(title.isEmpty());
            //bool exiting(newTitle.isEmpty());

            title = newTitle;
            // FIXME should be done differently
            //sliver->setToolTip(QString("<strong style='white-space:pre; font-size:14pt'>%1</strong><br /><em>Current task</em>").arg(title));
            emit titleChanged(title);
        }
    }




    Sliver::Sliver(QWidget * parent)
        : QFrame(parent), d(new SliverPrivate(this))
    {
        d->init();
    }

    Sliver::Sliver(Qt::Orientation orientation, QWidget * parent)
        : QFrame(parent), d(new SliverPrivate(this))
    {
        d->init();
    }

    void Sliver::clearTitle()
    {
        setTitle(QString());
    }

    void Sliver::setTaskColor(const QColor & color)
    {
        d->setTaskColor(color);
    }

    void Sliver::setTitle(const QString & title)
    {
        d->setTitle(title);
    }

    QColor Sliver::taskColor() const
    {
        return d->taskColor;
    }

    QString Sliver::title() const
    {
        return d->title;
    }

}
