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

#ifndef GRAFFITI_GRAPHICSFLIPWIDGET_H
#define GRAFFITI_GRAPHICSFLIPWIDGET_H

#include <graffiti/config.h>

#include <QGraphicsItem>
#include <QObject>
#include <QTimeLine>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
class QGraphicsProxyWidget;
QT_END_NAMESPACE;

namespace Graffiti
{

    class LIBGRAFFITI_API GraphicsFlipWidget : public QObject, public QGraphicsItem
    {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

    public:
        GraphicsFlipWidget(QWidget * frontWidget, QWidget * backWidget, QGraphicsItem * parent = 0);
        ~GraphicsFlipWidget();

        QWidget * backWidget() const;
        QRectF boundingRect () const;
        QWidget * frontWidget() const;
        QWidget * hiddenWidget() const;
        bool isFlipped() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *);
        QWidget * visibleWidget() const;

    public slots:
        void flip();

    protected slots:
        void animateFlip(qreal value);
        void regainFocus();

    signals:
        void flipped();

    protected:
        void keyPressEvent(QKeyEvent * event);
        QGraphicsProxyWidget * hiddenProxyWidget() const;
        qreal transitionPoint() const;
        QGraphicsProxyWidget * visibleProxyWidget() const;



    private:
        QGraphicsProxyWidget * frontProxyWidget;
        QGraphicsProxyWidget * backProxyWidget;
        QTimeLine flipTimeLine;

    };

}

#endif // GRAPHICSFLIPWIDGET_H
