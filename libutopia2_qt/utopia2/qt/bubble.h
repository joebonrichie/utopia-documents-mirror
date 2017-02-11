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

#ifndef UTOPIA2_QT_BUBBLE_H
#define UTOPIA2_QT_BUBBLE_H

#include <utopia2/qt/config.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QPolygon>
#include <boost/scoped_ptr.hpp>
#include <QStatusBar>
#include <QWidget>

#include <QtDebug>

#define PIPLENGTH 20
#define PIPWIDTH 28

namespace Utopia
{

    typedef enum
    {
        NoCorners         = 0x00,
        TopLeftCorner     = 0x01,
        TopRightCorner    = 0x02,
        BottomLeftCorner  = 0x04,
        BottomRightCorner = 0x08,
        TopCorners        = TopLeftCorner | TopRightCorner,
        BottomCorners     = BottomLeftCorner | BottomRightCorner,
        RightCorners      = TopRightCorner | BottomRightCorner,
        LeftCorners       = TopLeftCorner | BottomLeftCorner,
        AllCorners        = TopCorners | BottomCorners
    } Corner;
    Q_DECLARE_FLAGS(Corners, Corner);

    typedef enum
    {
        NoCallout,
        LeftCallout,
        TopCallout,
        RightCallout,
        BottomCallout
    } CalloutSide;
    Q_ENUMS(CalloutSide);



    template< typename WidgetBase >
    class Bubble : public WidgetBase
    {
    public:
        Bubble(QWidget * parent = 0, Qt::WindowFlags f = 0)
            : WidgetBase(parent, f | Qt::FramelessWindowHint)
        {
            WidgetBase::setAttribute(Qt::WA_TranslucentBackground, true);

            _d.titleLabel = new QLabel(this);
            _d.titleLabel->setObjectName("title");
            _d.titleLabel->setAlignment(Qt::AlignCenter);
            _d.titleLabel->setMargin(2);

            setBubbleBackground(QColor(60, 60, 60));
            setBubbleBorder(QColor(0, 0, 0, 0));
            setBubbleTitleBarBackground(QColor(20, 20, 20));
            setBubbleTitleBarBorder(QColor(0, 0, 0, 0));

            //_d.closeButton = new QPushButton(this);

            calculateBubbleRect();
        }

        ~Bubble()
        {}

        void adjust(QWidget *target)
        {
            const QObjectList children = target->children();
            for (int i = 0; i < children.size(); ++i) {
                QWidget *child = static_cast<QWidget*>(children.at(i));
                if (child->isWidgetType() &&
                    child != _d.titleLabel &&
                    child != _d.statusBar &&
                    child != _d.closeButton)
                    child->adjustSize();
            }
            target->adjustSize();
        }

        QBrush bubbleBackground() const
        {
            return _d.bubbleBackground;
        }

        QBrush bubbleBorder() const
        {
            return _d.bubbleBorder;
        }

        QBrush bubbleTitleBarBackground() const
        {
            return _d.bubbleTitleBarBackground;
        }

        QBrush bubbleTitleBarBorder() const
        {
            return _d.bubbleTitleBarBorder;
        }

        int calloutPosition() const
        {
            return _d.calloutPosition;
        }

        CalloutSide calloutSide() const
        {
            return _d.calloutSide;
        }

        QMargins contentsMargins() const
        {
            return _d.contentsMargins;
        }

        Corners corners() const
        {
            return _d.corners;
        }

        bool event(QEvent * event)
        {
            if (isPopup() && event->type() == QEvent::ActivationChange && !WidgetBase::isActiveWindow()) {
                WidgetBase::deleteLater();
            }
            return WidgetBase::event(event);
        }

        void getContentsMargins(int * left, int * top, int * right, int * bottom) const
        {
            *left = _d.contentsMargins.left();
            *top = _d.contentsMargins.top();
            *right = _d.contentsMargins.right();
            *bottom = _d.contentsMargins.bottom();
        }

        bool isCloseButtonEnabled() const
        {
            return _d.closeButtonEnabled;
        }

        bool isPopup() const
        {
            return _d.isPopup;
        }

        int radius() const
        {
            return _d.radius;
        }

        static void sendResizeEvents(QWidget *target)
        {
            QResizeEvent e(target->size(), QSize());
            QApplication::sendEvent(target, &e);

            const QObjectList children = target->children();
            for (int i = 0; i < children.size(); ++i) {
                QWidget *child = static_cast<QWidget*>(children.at(i));
                if (child->isWidgetType() && !child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent))
                    sendResizeEvents(child);
            }
        }

        void setBubbleBackground(const QBrush & brush)
        {
            _d.bubbleBackground = brush;
            WidgetBase::update();
        }

        void setBubbleBorder(const QBrush & brush)
        {
            _d.bubbleBorder = brush;
            WidgetBase::update();
        }

        void setBubbleTitleBarBackground(const QBrush & brush)
        {
            _d.bubbleTitleBarBackground = brush;
            WidgetBase::update();
        }

        void setBubbleTitleBarBorder(const QBrush & brush)
        {
            _d.bubbleTitleBarBorder = brush;
            WidgetBase::update();
        }

        void setCalloutPosition(int calloutPosition)
        {
            if (_d.calloutPosition != calloutPosition) {
                _d.calloutPosition = calloutPosition;
                calculateBubbleRect();
            }
        }

        void setCalloutSide(CalloutSide calloutSide)
        {
            if (_d.calloutSide != calloutSide) {
                _d.calloutSide = calloutSide;
                calculateBubbleRect();
            }
        }

        void setCloseButtonEnabled(bool enabled)
        {
            _d.closeButtonEnabled = enabled;
        }

        void setContentsMargins(int left, int top, int right, int bottom)
        {
            setContentsMargins(QMargins(left, top, right, bottom));
        }

        void setContentsMargins(const QMargins & margins)
        {
            _d.contentsMargins = margins;
            calculateBubbleRect();
        }

        void setCorners(Corners corners)
        {
            _d.corners = corners;
        }

        void setPopup(bool popup)
        {
            _d.isPopup = popup;
        }

        void setRadius(int radius)
        {
            _d.radius = radius;
        }

        void setStatusBar(QStatusBar * statusBar)
        {
            if (_d.statusBar) { delete _d.statusBar; }
            statusBar->setParent((WidgetBase *) this);
            statusBar->setFixedHeight(22);
            _d.statusBar = statusBar;
            calculateBubbleRect();
        }

        void show(const QPoint & globalPos, Qt::Orientation orientation = Qt::Vertical)
        {
            _d.showGlobalPos = globalPos;
            _d.showOrientation = orientation;
            //adjust((WidgetBase *) this);
            //WidgetBase::adjustSize();

            if (!_d.showGlobalPos.isNull()) {
                //qDebug() << "======" << WidgetBase::geometry();
                QRect available;
                QPoint topLeft;
                if (WidgetBase::window() == this) {
                    available = QRect(QApplication::desktop()->availableGeometry(_d.showGlobalPos));
                } else {
                    available = QRect(WidgetBase::window()->geometry()).adjusted(4, 4, -4, -4);
                }
                if (_d.showOrientation == Qt::Vertical) {
                    setCalloutPosition(radius() + PIPWIDTH / 2);
                    if (available.right() - WidgetBase::width() > _d.showGlobalPos.x() + 6) {
                        setCalloutSide(LeftCallout);
                        topLeft.setX(_d.showGlobalPos.x() + 6);
                    } else {
                        setCalloutSide(RightCallout);
                        topLeft.setX(_d.showGlobalPos.x() - WidgetBase::width() - 6);
                    }
                    topLeft.setY(_d.showGlobalPos.y() - calloutPosition());
                    int missing = topLeft.y() + WidgetBase::height() - available.bottom();
                    missing = qBound(0, missing, qMax(0, topLeft.y() - available.top()));
                    topLeft.setY(topLeft.y() - missing);
                    setCalloutPosition(calloutPosition() + missing);
                } else {
                    setCalloutPosition(radius() + PIPWIDTH / 2);
                    topLeft = QPoint(_d.showGlobalPos.x() - calloutPosition(), _d.showGlobalPos.y() + 6);
                    setCalloutSide(TopCallout);
                }

                if (WidgetBase::parent()) {
                    topLeft = WidgetBase::parentWidget()->mapFromGlobal(topLeft);
                }
                WidgetBase::move(topLeft);
            }

            show();

            _d.showGlobalPos = QPoint();
        }

        void showEvent(QShowEvent * event)
        {
        /*
            if (!_d.showGlobalPos.isNull()) {
                int left, top;
                QRect available(QApplication::desktop()->availableGeometry(_d.showGlobalPos));
                if (_d.showOrientation == Qt::Vertical) {
                    setCalloutPosition(radius() + PIPWIDTH / 2);
                    if (available.right() - WidgetBase::width() > _d.showGlobalPos.x() + 6) {
                        setCalloutSide(LeftCallout);
                        left = _d.showGlobalPos.x() + 6;
                    } else {
                        setCalloutSide(RightCallout);
                        left = _d.showGlobalPos.x() - WidgetBase::width() - 6;
                    }
                    top = _d.showGlobalPos.y() - calloutPosition();
                    int missing = top + WidgetBase::height() - available.bottom();
                    missing = qBound(0, missing, qMax(0, top - available.top()));
                    top -= missing;
                    setCalloutPosition(calloutPosition() + missing);
                } else {
                    setCalloutPosition(radius() + PIPWIDTH / 2);
                    left = _d.showGlobalPos.x() - calloutPosition();
                    top = _d.showGlobalPos.y() + 6;
                    setCalloutSide(TopCallout);
                }

                WidgetBase::move(left, top);
            }

            _d.showGlobalPos = QPoint();
        */
        }

        QStatusBar * statusBar() const
        {
            return _d.statusBar;
        }

        using WidgetBase::show;

    protected:
        void changeEvent(QEvent * event)
        {
            if (event->type() == QEvent::WindowTitleChange)
            {
                QString newTitle(WidgetBase::windowTitle());
                elideTitle(newTitle);
                _d.titleLabel->setVisible(!newTitle.isEmpty());
                _d.titleLabel->adjustSize();
                calculateBubbleRect();
            }

            WidgetBase::changeEvent(event);
        }

        void elideTitle(const QString & title)
        {
            QFontMetrics fm(_d.titleLabel->fontMetrics());
            QString elidedTitle(fm.elidedText(title, Qt::ElideRight, _d.titleLabel->contentsRect().width()));
            _d.titleLabel->setText(elidedTitle);
        }

        void moveEvent(QMoveEvent * event)
        {
            return;
            /*
            qDebug() << _d.showGlobalPos << !_d.hideCallout << event->spontaneous() << event->oldPos() << event->pos();
            if (_d.showGlobalPos.isNull() && !_d.hideCallout && event->spontaneous() && event->oldPos() != event->pos()) {
                _d.hideCallout = true;
                _d.hiddenPadding = true;
                // HACK to make sure the window system realises the shape has changed!
                WidgetBase::resize(WidgetBase::size() + QSize(1, 1));
                calculateBubbleRect();
                event->accept();
            }
            */
        }

        void mouseMoveEvent(QMouseEvent * event)
        {
            if (_d.dragging) {
                WidgetBase::move(event->globalPos() - _d.dragMouseAnchor);
                event->accept();
            }
            WidgetBase::mouseMoveEvent(event);
        }

        void mousePressEvent(QMouseEvent * event)
        {
            if (event->button() == Qt::LeftButton && _d.calloutSide == NoCallout) {
                _d.dragging = true;
                _d.dragMouseAnchor = event->globalPos() - WidgetBase::frameGeometry().topLeft();
                event->accept();
            }
            WidgetBase::mousePressEvent(event);
        }

        void mouseReleaseEvent(QMouseEvent * event)
        {
            if (_d.dragging && event->button() == Qt::LeftButton) {
                _d.dragging = false;
                event->accept();
            }
            WidgetBase::mouseReleaseEvent(event);
        }

        void paintEvent(QPaintEvent * /*event*/)
        {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setOpacity(1.0);

            // Draw bubble background
            p.setPen(Qt::NoPen);
            p.setBrush(_d.bubbleBackground);
            p.drawPath(_d.bubblePath);

            // Draw bubble title bar background (optional)
            if (!WidgetBase::windowTitle().isEmpty()) {
                p.setPen(QPen(_d.bubbleTitleBarBorder, 1));
                p.setBrush(_d.bubbleTitleBarBackground);
                p.drawPath(_d.titlePath);
            }

            // Draw bubble border
            p.setPen(QPen(_d.bubbleBorder, 0));
            p.setBrush(Qt::NoBrush);
            p.drawPath(_d.outlinePath);
        }

        void resizeEvent(QResizeEvent * event)
        {
            if (_d.showGlobalPos.isNull() && !_d.hideCallout && event->spontaneous() && event->oldSize().isValid() && event->oldSize() != event->size())
            {
                _d.hideCallout = true;
            }
            elideTitle(WidgetBase::windowTitle());
            calculateBubbleRect();
        }

    protected:
        class BubblePrivate
        {
            public:
                BubblePrivate()
                : calloutSide(NoCallout),
                  calloutPosition(0),
                  closeButtonEnabled(true),
                  contentsMargins(0, 0, 0, 0),
                  corners(AllCorners),
                  radius(8),
                  hideCallout(false),
                  hiddenPadding(false),
                  statusBar(0),
                  dragging(false),
                  calloutCenter(0),
                  showOrientation(Qt::Vertical),
                  isPopup(false)
                {}

                CalloutSide calloutSide;
                int calloutPosition;
                bool closeButtonEnabled;
                QMargins contentsMargins;
                Corners corners;
                int radius;
                QBrush bubbleBackground;
                QBrush bubbleBorder;
                QBrush bubbleTitleBarBackground;
                QBrush bubbleTitleBarBorder;
                bool hideCallout;
                bool hiddenPadding;

                QLabel * titleLabel;
                QPushButton * closeButton;
                QStatusBar * statusBar;

                bool dragging;
                QPoint dragMouseAnchor;

                QPainterPath bubblePath;
                QPainterPath outlinePath;
                QPainterPath titlePath;

                QRectF bubbleRect;
                QRect contentsRect;
                int calloutCenter;

                QPoint showGlobalPos;
                Qt::Orientation showOrientation;

                bool isPopup;
        } _d;

        bool hasCorner(Corners corners)
        {
            if (_d.statusBar && (corners & BottomCorners))
            {
                return false;
            }
            else
            {
                return _d.corners & corners;
            }
        }

        void calculateBubbleRect(bool straightCallout = false)
        {
            elideTitle(WidgetBase::windowTitle());

            _d.bubblePath = QPainterPath();
            _d.titlePath = QPainterPath();

            // Calculate dimensions of the bubble itself
            _d.bubbleRect = QRectF(WidgetBase::rect()).adjusted(
                _d.calloutSide == LeftCallout ? PIPLENGTH : 0,
                _d.calloutSide == TopCallout ? PIPLENGTH : 0,
                _d.calloutSide == RightCallout ? -PIPLENGTH : 0,
                _d.calloutSide == BottomCallout ? -PIPLENGTH : 0
                ).adjusted(0.5, 0.5, -0.5, -0.5);
            if (_d.hiddenPadding)
            {
                _d.bubbleRect.adjust(0, 0, -1, -1);
            }

            // Generate path for bubble, rounding corners as needded
            _d.bubblePath.addRoundedRect(_d.bubbleRect, _d.radius, _d.radius);
            QPainterPath cornerPath;
            QRectF cornerSquare(0, 0, _d.radius, _d.radius);
            if (hasCorner(TopLeftCorner) == 0)
            {
                cornerSquare.moveTopLeft(_d.bubbleRect.topLeft());
                cornerPath.addRect(cornerSquare);
            }
            if (hasCorner(TopRightCorner) == 0)
            {
                cornerSquare.moveTopRight(_d.bubbleRect.topRight());
                cornerPath.addRect(cornerSquare);
            }
            if (hasCorner(BottomRightCorner) == 0)
            {
                cornerSquare.moveBottomRight(_d.bubbleRect.bottomRight());
                cornerPath.addRect(cornerSquare);
            }
            if (hasCorner(BottomLeftCorner) == 0)
            {
                cornerSquare.moveBottomLeft(_d.bubbleRect.bottomLeft());
                cornerPath.addRect(cornerSquare);
            }
            _d.bubblePath += cornerPath;
            _d.outlinePath = _d.bubblePath;

            // Generate title path if needed
            if (!WidgetBase::windowTitle().isEmpty())
            {
                _d.titlePath.addRect(_d.bubbleRect.adjusted(0, 0, 0, - _d.bubbleRect.height() + _d.titleLabel->height()));
                _d.titlePath = _d.titlePath.intersected(_d.bubblePath);
                _d.bubblePath -= _d.titlePath;
            }

            // Generate callout pip
            if (!_d.hideCallout && _d.calloutSide != NoCallout)
            {
                // Calculate callout position
                int extent = (_d.calloutSide == TopCallout ||
                              _d.calloutSide == BottomCallout) ? WidgetBase::width() : WidgetBase::height();
                int titleBuffer = 0;
                if (!WidgetBase::windowTitle().isEmpty() &&
                    (_d.calloutSide == LeftCallout || _d.calloutSide == RightCallout))
                {
                    titleBuffer = _d.titleLabel->height();
                }
                _d.calloutPosition = qBound(0, _d.calloutPosition, extent);
                _d.calloutCenter = qBound(_d.radius + qRound(PIPWIDTH * 0.5) + titleBuffer, _d.calloutPosition, extent - qRound(PIPWIDTH * 0.5) - _d.radius);
                if (straightCallout)
                {
                    _d.calloutPosition = _d.calloutCenter;
                }

                // Pip path
                QPainterPath calloutPath;
                switch (_d.calloutSide)
                {
                case LeftCallout:
                    calloutPath.moveTo(QPointF(_d.bubbleRect.left(), _d.calloutCenter + PIPWIDTH / 2));
                    calloutPath.quadTo(QPointF(_d.bubbleRect.left(), _d.calloutCenter + PIPWIDTH / 4),
                                       QPointF(0, _d.calloutPosition));
                    calloutPath.quadTo(QPointF(_d.bubbleRect.left(), _d.calloutCenter - PIPWIDTH / 4),
                                       QPointF(_d.bubbleRect.left(), _d.calloutCenter - PIPWIDTH / 2));
                    calloutPath.closeSubpath();
                    break;
                case TopCallout:
                    calloutPath.moveTo(QPointF(_d.calloutCenter - PIPWIDTH / 2, _d.bubbleRect.top()));
                    calloutPath.quadTo(QPointF(_d.calloutCenter - PIPWIDTH / 4, _d.bubbleRect.top()),
                                       QPointF(_d.calloutPosition, 0));
                    calloutPath.quadTo(QPointF(_d.calloutCenter + PIPWIDTH / 4, _d.bubbleRect.top()),
                                       QPointF(_d.calloutCenter + PIPWIDTH / 2, _d.bubbleRect.top()));
                    calloutPath.closeSubpath();
                    break;
                case RightCallout:
                    calloutPath.moveTo(QPointF(_d.bubbleRect.right(), _d.calloutCenter - PIPWIDTH / 2));
                    calloutPath.quadTo(QPointF(_d.bubbleRect.right(), _d.calloutCenter - PIPWIDTH / 4),
                                       QPointF(WidgetBase::width(), _d.calloutPosition));
                    calloutPath.quadTo(QPointF(_d.bubbleRect.right(), _d.calloutCenter + PIPWIDTH / 4),
                                       QPointF(_d.bubbleRect.right(), _d.calloutCenter + PIPWIDTH / 2));
                    calloutPath.closeSubpath();
                    break;
                case BottomCallout:
                    calloutPath.moveTo(QPointF(_d.calloutCenter + PIPWIDTH / 2, _d.bubbleRect.bottom()));
                    calloutPath.quadTo(QPointF(_d.calloutCenter + PIPWIDTH / 4, _d.bubbleRect.bottom()),
                                       QPointF(_d.calloutPosition, WidgetBase::height()));
                    calloutPath.quadTo(QPointF(_d.calloutCenter - PIPWIDTH / 4, _d.bubbleRect.bottom()),
                                       QPointF(_d.calloutCenter - PIPWIDTH / 2, _d.bubbleRect.bottom()));
                    calloutPath.closeSubpath();
                    break;
                default:
                    break;
                }
                _d.bubblePath += calloutPath;
                _d.bubblePath.setFillRule(Qt::WindingFill);
                _d.bubblePath = _d.bubblePath.simplified();
                _d.outlinePath += calloutPath;
                _d.outlinePath.setFillRule(Qt::WindingFill);
                _d.outlinePath = _d.outlinePath.simplified();
                if (_d.calloutSide == TopCallout && !WidgetBase::windowTitle().isEmpty())
                {
                    _d.titlePath += calloutPath;
                    _d.titlePath.setFillRule(Qt::WindingFill);
                    _d.titlePath = _d.titlePath.simplified();
                }
            }

            // Calculate the internal dimensions of bubble, minus title and status bar
            _d.contentsRect = _d.bubbleRect.toAlignedRect().adjusted(
                0,
                WidgetBase::windowTitle().isEmpty() ? 0 : _d.titleLabel->height(),
                0,
                _d.statusBar ? -_d.statusBar->height() : 0
                );

            // Position sub controls
            _d.titleLabel->setGeometry(_d.bubbleRect.left() + _d.radius,
                                       _d.bubbleRect.top(),
                                       _d.bubbleRect.width() - (2 * _d.radius),
                                       _d.titleLabel->height());
            if (_d.statusBar)
            {
                _d.statusBar->setGeometry(_d.bubbleRect.left(),
                                          _d.bubbleRect.bottom() - _d.statusBar->height() + 1,
                                          _d.bubbleRect.width(),
                                          _d.statusBar->height());
                _d.statusBar->setSizeGripEnabled(true);
            }

            // QWidget margins (callout + this bubble's margins + title?)
            QMargins margins(_d.contentsMargins.left() + (_d.calloutSide == LeftCallout ? PIPLENGTH : 0),
                             _d.contentsMargins.top() + (_d.calloutSide == TopCallout ? PIPLENGTH : 0),
                             _d.contentsMargins.right() + (_d.calloutSide == RightCallout ? PIPLENGTH : 0),
                             _d.contentsMargins.bottom() + (_d.calloutSide == BottomCallout ? PIPLENGTH : 0));
            if (!WidgetBase::windowTitle().isEmpty()) { margins.setTop(margins.top() + _d.titleLabel->height()); }
            else if (hasCorner(TopCorners)) { margins.setTop(margins.top() + _d.radius); }
            if (_d.statusBar) { margins.setBottom(margins.bottom() + _d.statusBar->height()); }
            else if (hasCorner(BottomCorners)) { margins.setBottom(margins.bottom() + _d.radius); }
            if (hasCorner(LeftCorners)) { margins.setLeft(margins.left() + _d.radius); }
            if (hasCorner(RightCorners)) { margins.setRight(margins.right() + _d.radius); }
            WidgetBase::setContentsMargins(margins);

            // Reflect changes on screen
            WidgetBase::update();
        }
    };

    class BubbleWidget : public Bubble< QWidget >
    {
        Q_OBJECT
        Q_PROPERTY(QBrush bubbleBackground
                   READ bubbleBackground
                   WRITE setBubbleBackground
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleBorder
                   READ bubbleBorder
                   WRITE setBubbleBorder
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleTitleBarBackground
                   READ bubbleTitleBarBackground
                   WRITE setBubbleTitleBarBackground
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleTitleBarBorder
                   READ bubbleTitleBarBorder
                   WRITE setBubbleTitleBarBorder
                   DESIGNABLE true)

    public:
        BubbleWidget(QWidget * parent = 0, Qt::WindowFlags f = 0)
            : Bubble< QWidget >(parent, f)
        {}
    };

    class BubbleDialog : public Bubble< QDialog >
    {
        Q_OBJECT
        Q_PROPERTY(QBrush bubbleBackground
                   READ bubbleBackground
                   WRITE setBubbleBackground
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleBorder
                   READ bubbleBorder
                   WRITE setBubbleBorder
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleTitleBarBackground
                   READ bubbleTitleBarBackground
                   WRITE setBubbleTitleBarBackground
                   DESIGNABLE true)
        Q_PROPERTY(QBrush bubbleTitleBarBorder
                   READ bubbleTitleBarBorder
                   WRITE setBubbleTitleBarBorder
                   DESIGNABLE true)

    public:
        BubbleDialog(QWidget * parent = 0, Qt::WindowFlags f = 0)
            : Bubble< QDialog >(parent, f | Qt::Dialog)
        {}
    };

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Utopia::Corners);

#endif // UTOPIA2_QT_BUBBLEWIDGET_H
