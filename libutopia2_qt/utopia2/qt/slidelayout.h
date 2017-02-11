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

#ifndef UTOPIA_SLIDELAYOUT_H
#define UTOPIA_SLIDELAYOUT_H

#include <utopia2/qt/config.h>

#include <QLayout>

namespace Utopia
{

    class SlideLayoutPrivate;
    class SlideLayout : public QLayout
    {
        Q_OBJECT
        Q_ENUMS(StackDirection)
        Q_PROPERTY(int slideDuration
                   READ slideDuration
                   WRITE setSlideDuration
                   DESIGNABLE true)
        Q_PROPERTY(int updateInterval
                   READ updateInterval
                   WRITE setUpdateInterval
                   DESIGNABLE true)

    public:
        enum StackDirection { StackLeft, StackRight, StackAuto };

        SlideLayout(QWidget * parent);
        SlideLayout();
        SlideLayout(StackDirection stackDirection, QWidget * parent);
        SlideLayout(StackDirection stackDirection);

        void addItem(QLayoutItem * item);
        void addWidget(QWidget * widget, const QString & id = QString());
        int count() const;
        QLayoutItem * itemAt(int index) const;
        QSize minimumSize() const;
        void setGeometry(const QRect & rect);
        void setSlideDuration(int duration);
        void setUpdateInterval(int msecs);
        QSize sizeHint() const;
        int slideDuration() const;
        QLayoutItem * takeAt(int);
        QWidget * top() const;
        int updateInterval() const;

    public slots:
        void pop(bool animate = true);
        void push(const QString & id, bool animate = true);
        void push(QWidget * widget, bool animate = true);

    signals:
        void animated();
        void widgetChanged(QWidget * widget);

    private:
        SlideLayoutPrivate * d;
    };

}

#endif // UTOPIA_SLIDELAYOUT_H
