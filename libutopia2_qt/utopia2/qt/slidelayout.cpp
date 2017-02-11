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

#include <utopia2/qt/slidelayout_p.h>
#include <utopia2/qt/slidelayout.h>

#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <QWidgetItem>

#include <QDebug>

#define UTOPIA_SLIDELAYOUT_ID_PROPERTY "__u_paneId"

namespace Utopia
{

    SlideLayoutPrivate::SlideLayoutPrivate(SlideLayout * layout, SlideLayout::StackDirection direction)
        : QObject(layout),
          layout(layout),
          stackDirection(direction),
          leftPage(0),
          rightPage(0),
          currentPage(0),
          timeLine(300)
    {
        if (stackDirection == SlideLayout::StackAuto)
            // FIXME this should be according to the layout direction
            stackDirection = SlideLayout::StackRight;

        timeLine.setUpdateInterval(10);

        connect(&timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(animate(qreal)));
        connect(&timeLine, SIGNAL(finished()), this, SLOT(animationFinished()));
        connect(this, SIGNAL(animated()), layout, SIGNAL(animated()));
        connect(this, SIGNAL(widgetChanged(QWidget *)), layout, SIGNAL(widgetChanged(QWidget *)));

    }

    void SlideLayoutPrivate::animate(qreal value)
    {
        if (QWidget * widget = layout->parentWidget()) {
            int divider(qRound(widget->width() * value));
            if (leftPage) {
                leftPage->move(divider - widget->width(), 0);
            }
            if (rightPage) {
                rightPage->move(divider, 0);
            }
            emit animated();
        }
    }

    void SlideLayoutPrivate::animationFinished()
    {
        // Hide unused widgets
        if (timeLine.direction() == QTimeLine::Forward) {
            if (rightPage) {
                rightPage->hide();
            }
            currentPage = leftPage;
        } else {
            if (leftPage) {
                leftPage->hide();
            }
            currentPage = rightPage;
        }

        // Set off new animation if need be
        if (currentPage != (pageStack.isEmpty() ? 0 : pageStack.top())) {
            QTimer::singleShot(0, this, SLOT(startAnimation()));
        }
    }

    void SlideLayoutPrivate::connectPanes(QWidget * widget)
    {
        static const char * pushPaneSignal = SIGNAL(pushSlideLayoutPane(const QString &));
        static const char * pushPaneSlot = SLOT(push(const QString &));
        static const char * popPaneSignal = SIGNAL(popSlideLayoutPane());
        static const char * popPaneSlot = SLOT(pop());

        const QMetaObject * mObj = widget->metaObject();
        int pushPaneIndex = mObj->indexOfSignal(pushPaneSignal);
        int popPaneIndex = mObj->indexOfSignal(popPaneSignal);

        if (pushPaneIndex != -1) {
            connect(widget, pushPaneSignal, layout, pushPaneSlot);
        }
        if (popPaneIndex != -1) {
            connect(widget, popPaneSignal, layout, popPaneSlot);
        }
    }

    void SlideLayoutPrivate::startAnimation(bool doAnimate)
    {
        if (timeLine.state() == QTimeLine::NotRunning) {
            QWidget * fromPage = currentPage;
            QWidget * toPage = pageStack.isEmpty() ? (QWidget *) 0 : pageStack.top();
            bool pushing(fromPage == 0 || pageStack.contains(fromPage));
            if ((pushing && stackDirection == SlideLayout::StackRight) ||
                (!pushing && stackDirection == SlideLayout::StackLeft)) {
                leftPage = fromPage;
                rightPage = toPage;
                timeLine.setDirection(QTimeLine::Backward);
            } else {
                leftPage = toPage;
                rightPage = fromPage;
                timeLine.setDirection(QTimeLine::Forward);
            }
            if (fromPage) {
                fromPage->show();
            }
            if (toPage) {
                toPage->show();
            }

            // Either animate, or jump directly to the end if no animation is needed
            if (doAnimate) {
                timeLine.start();
                animate(timeLine.currentValue());
            } else {
                if (timeLine.direction() == QTimeLine::Forward) {
                    animate(1.0);
                    animationFinished();
                } else if (timeLine.direction() == QTimeLine::Backward) {
                    animate(0.0);
                    animationFinished();
                }
            }

            emit widgetChanged(toPage);
        }
    }




    SlideLayout::SlideLayout(QWidget * parent)
        : QLayout(parent), d(new SlideLayoutPrivate(this))
    {}

    SlideLayout::SlideLayout()
        : QLayout(), d(new SlideLayoutPrivate(this))
    {}

    SlideLayout::SlideLayout(SlideLayout::StackDirection stackDirection, QWidget * parent)
        : QLayout(parent), d(new SlideLayoutPrivate(this, stackDirection))
    {}

    SlideLayout::SlideLayout(SlideLayout::StackDirection stackDirection)
        : QLayout(), d(new SlideLayoutPrivate(this, stackDirection))
    {}

    void SlideLayout::addItem(QLayoutItem * item)
    {
        if (QWidget * widget = item->widget()) {
            addWidget(widget);
            delete item;
        } else {
            qWarning("SlideLayout::addItem: Only widgets can be added");
        }
    }

    void SlideLayout::addWidget(QWidget * widget, const QString & id)
    {
        if (indexOf(widget) == -1) {
            widget->hide();
            addChildWidget(widget);
            d->pagePool << new QWidgetItem(widget);
            widget->setProperty(UTOPIA_SLIDELAYOUT_ID_PROPERTY, id);
            d->connectPanes(widget);
        }
    }

    int SlideLayout::count() const
    {
        return d->pagePool.size();
    }

    QLayoutItem * SlideLayout::itemAt(int index) const
    {
        return d->pagePool.value(index);
    }

    QSize SlideLayout::minimumSize() const
    {
        QSize size(0, 0);

        QListIterator< QWidgetItem * > iter(d->pagePool);
        while (iter.hasNext()) {
            if (QWidget * widget = iter.next()->widget()) {
                QSize widgetSize(widget->minimumSizeHint());
                if (widget->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
                    widgetSize.setWidth(0);
                }
                if (widget->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
                    widgetSize.setHeight(0);
                }
                size = size.expandedTo(widgetSize);
            }
        }

        return size;
    }

    void SlideLayout::pop(bool animate)
    {
        if (!d->pageStack.isEmpty()) {
            d->pageStack.pop();
            d->startAnimation(animate);
        } else {
            qWarning("SlideLayout::pop: Cannot pop from an empty stack");
        }
    }

    void SlideLayout::push(const QString & id, bool animate)
    {
        if (!id.isEmpty()) {
            QListIterator< QWidgetItem * > iter(d->pagePool);
            while (iter.hasNext()) {
                QWidget * widget = iter.next()->widget();
                if (widget->property(UTOPIA_SLIDELAYOUT_ID_PROPERTY).toString() == id) {
                    push(widget, animate);
                    return;
                }
            }
        }

        qWarning() << "SlideLayout::push: Unknown pane id:" << id;
    }

    void SlideLayout::push(QWidget * widget, bool animate)
    {
        if (indexOf(widget) != -1 && !d->pageStack.contains(widget)) {
            d->pageStack.push(widget);
            d->startAnimation(animate);
        } else {
            qWarning("SlideLayout::push: Only added unpushed widgets can be pushed");
        }
    }

    void SlideLayout::setGeometry(const QRect & rect)
    {
        QListIterator< QWidgetItem * > iter(d->pagePool);
        while (iter.hasNext()) {
            // Take into account a resize while animating (don't set geometry, but rather
            // just resize, and maybe reposition (if leftPage)
            if (QWidget * widget = iter.next()->widget()) {
                if (d->timeLine.state() == QTimeLine::Running && widget == d->leftPage) {
                    widget->move(widget->geometry().topRight() - QPoint(rect.width(), 0));
                }
                widget->resize(rect.size());
            }
        }
    }

    void SlideLayout::setSlideDuration(int duration)
    {
        d->timeLine.setDuration(duration);
    }

    void SlideLayout::setUpdateInterval(int msecs)
    {
        d->timeLine.setUpdateInterval(msecs);
    }

    QSize SlideLayout::sizeHint() const
    {
        QSize hint(0, 0);

        QListIterator< QWidgetItem * > iter(d->pagePool);
        while (iter.hasNext()) {
            if (QWidget * widget = iter.next()->widget()) {
                QSize widgetSize(widget->sizeHint());
                if (widget->sizePolicy().horizontalPolicy() == QSizePolicy::Ignored) {
                    widgetSize.setWidth(0);
                }
                if (widget->sizePolicy().verticalPolicy() == QSizePolicy::Ignored) {
                    widgetSize.setHeight(0);
                }
                hint = hint.expandedTo(widgetSize);
            }
        }

        return hint;
    }

    int SlideLayout::slideDuration() const
    {
        return d->timeLine.duration();
    }

    QLayoutItem * SlideLayout::takeAt(int index)
    {
        if (index < 0 || index >= d->pagePool.size()) {
            return 0;
        } else {
            return d->pagePool.takeAt(index);
        }
    }

    QWidget * SlideLayout::top() const
    {
        return d->pageStack.isEmpty() ? (QWidget *) 0 : d->pageStack.top();
    }

    int SlideLayout::updateInterval() const
    {
        return d->timeLine.updateInterval();
    }

}
