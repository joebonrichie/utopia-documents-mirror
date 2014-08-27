/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <papyro/tabbar.h>
#include <papyro/tabbar_p.h>

#include <QEvent>
#include <QHelpEvent>
#include <QImage>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QPainter>
#include <QToolTip>
#include <QVariant>
#include <QWheelEvent>

#include <QDebug>

namespace Papyro
{

    TabBarPrivate::TabBarPrivate(TabBar * tabBar)
        : QObject(tabBar), tabBar(tabBar), currentIndex(-1),
          activeImage(":/images/tab-west-active.png"),
          inactiveImage(":/images/tab-west-inactive.png"),
          hoverImage(":/images/tab-west-hover.png"), assetScale(2), minTabSize(100),
          maxTabSize(200), tabSpacing(-16), tabPadding(4), tabFading(10), tabMargin(6),
          spinnerSize(14), position(0), extent(0), tabUnderMouse(-1),
          tabButtonPressed(-1), tabButtonUnderMouse(-1)
    {
        // One pixel for the inner part of the tab, the rest for each tab edge
        tabEdgeSize = (activeImage.height() - 1) / (2 * assetScale);

        wheelDelay.setInterval(100);
        wheelDelay.setSingleShot(true);

        animationTimer.setInterval(40);
        connect(&animationTimer, SIGNAL(timeout()), tabBar, SLOT(update()));

        tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

        // FIXME use minTabSize to squash tabs down a bit
    }

    TabBarPrivate::~TabBarPrivate()
    {}

    int TabBarPrivate::getCurrentIndex() const
    {
        return targets.isEmpty() ? -1 : qBound(0, currentIndex, targets.size() - 1);
    }

    int TabBarPrivate::getPosition() const
    {
        int preferred = 0;
        if (getCurrentIndex() > 0) {
            const TabData * data = tabData(getCurrentIndex());
            int from = data->offset + data->size - tabBar->height() + tabMargin + tabFading;
            int to = data->offset - tabMargin;
            preferred = qBound(qMin(from, to), position, to);
        }
        return qBound(0, preferred, qMax(0, extent - tabBar->height()));
    }

    int TabBarPrivate::getTabOffset(int index) const
    {
        if (const TabData * data = tabData(index)) {
            return data->offset;
        }
        return 0;
    }

    QRect TabBarPrivate::getTabButtonRect(int index) const
    {
        if (const TabData * data = tabData(index)) {
            return QRect(1 + (activeImage.width() / assetScale - spinnerSize) / 2, data->offset + tabEdgeSize, spinnerSize, spinnerSize);
        } else {
            return QRect();
        }
    }

    QRect TabBarPrivate::getTabRect(int index) const
    {
        return getTabRect(tabData(index));
    }

    QRect TabBarPrivate::getTabRect(const TabData * data) const
    {
        if (data) {
            return QRect(0, data->offset - getPosition(), activeImage.width(), data->size);
        } else {
            return QRect();
        }
    }

    int TabBarPrivate::getTabSize(int index) const
    {
        if (const TabData * data = tabData(index)) {
            return data->size;
        }
        return 0;
    }

    void TabBarPrivate::paintTab(QPainter * painter, int index)
    {
        painter->save();

        const TabData & data = targets.at(index);
        bool active = (index == getCurrentIndex());
        bool hover = (index == tabUnderMouse);
        QPixmap pixmap(active ? activeImage : hover ? hoverImage : inactiveImage);
        QRect target, source;

        // Draw the tab's background
        target = QRect(0, data.offset, pixmap.width() / assetScale, tabEdgeSize);
        source = QRect(0, 0, pixmap.width(), tabEdgeSize * assetScale);
        painter->drawPixmap(target, pixmap, source);

        target = QRect(0, data.offset + tabEdgeSize, pixmap.width() / assetScale, data.size - 2 * tabEdgeSize);
        source = QRect(0, tabEdgeSize * assetScale, pixmap.width(), pixmap.height() - 2 * tabEdgeSize * assetScale);
        painter->drawPixmap(target, pixmap, source);

        target = QRect(0, data.offset + data.size - tabEdgeSize, pixmap.width() / assetScale, tabEdgeSize);
        source = QRect(0, pixmap.height() - tabEdgeSize * assetScale, pixmap.width(), tabEdgeSize * assetScale);
        painter->drawPixmap(target, pixmap, source);

        // Draw the tab's title text
        target = QRect(0, 0, data.size - 2 * tabEdgeSize - 2 * tabPadding, pixmap.width() / assetScale);
        if (!target.isEmpty()) {
            painter->save();
            painter->translate(0, data.offset + data.size - tabEdgeSize - tabPadding);
            painter->rotate(-90);
            QPixmap textPixmap(target.width() * assetScale, target.height() * assetScale);
            textPixmap.fill(Qt::transparent);
            {
                QPainter textPainter(&textPixmap);
                textPainter.setRenderHint(QPainter::Antialiasing, true);
                textPainter.setRenderHint(QPainter::TextAntialiasing, true);
                textPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                if (data.error) {
                    textPainter.setPen(QColor(200, 0, 0));
                }
                textPainter.scale((qreal) assetScale, (qreal) assetScale);
                target.setSize(target.size());
                textPainter.drawText(target, Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignLeft, data.title);
            }
            painter->scale(1 / (qreal) assetScale, 1 / (qreal) assetScale);
            painter->drawPixmap(0, 0, textPixmap);
            painter->restore();
        }

        // The rectangle into which we place the spinner / close button
        target = getTabButtonRect(index);
        bool hoverClose = target.contains(hoverPos + QPoint(0, getPosition()));
        bool pressClose = tabButtonPressed == index;

        if (hoverClose || pressClose || !data.busy) {
            painter->save();

            static const qreal crossLength = assetScale * 4.0 / 1.4142;
            static const qreal circleRadius = assetScale * 7;

            QPointF center(QRectF(target).center());
            QImage cross(target.size() * assetScale, QImage::Format_ARGB32_Premultiplied);
            cross.fill(Qt::transparent);
            {
                QPointF center(QRectF(cross.rect()).center());
                QPainter painter(&cross);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(QPen(Qt::black, 1.4142 * assetScale));
                painter.drawLine(center + QPointF(-crossLength, -crossLength), center + QPointF(crossLength, crossLength));
                painter.drawLine(center + QPointF(-crossLength, crossLength), center + QPointF(crossLength, -crossLength));
            }

            // Draw a close button
            if (hoverClose || pressClose) {
                QImage pixmap(target.size() * assetScale, QImage::Format_ARGB32_Premultiplied);
                pixmap.fill(Qt::transparent);
                {
                    QPointF center(QRectF(pixmap.rect()).center());
                    QPainter painter(&pixmap);
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(Qt::black);
                    painter.drawEllipse(center, circleRadius, circleRadius);
                    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                    painter.drawImage(QPoint(0, 0), cross);
                }
                painter->setOpacity(hoverClose && pressClose ? 0.6 : 0.4);
                painter->drawImage(target, pixmap);
            } else {
                painter->setOpacity(0.4);
                painter->drawImage(target, cross);
            }

            painter->restore();
        } else {
            painter->save();

            // Draw the tab's progress spinner

            // Clip an internal circle
            QPainterPath clip;
            clip.addRect(tabBar->rect().translated(0, getPosition()));
            qreal width = spinnerSize / 4.0;
            clip.addEllipse(target.adjusted(width, width, -width, -width));
            painter->setClipPath(clip);

            painter->setPen(Qt::NoPen);
            if (data.progress >= 0.0) {
                painter->setBrush(QColor(0, 0, 0, 20));
                painter->drawEllipse(target);
            }
            painter->setOpacity(0.4);
            painter->setBrush(Qt::black);
            int startAngle, sweepAngle;
            if (data.progress < 0.0) {
                startAngle = -(data.time.elapsed() * 7 % (360 * 16));
                sweepAngle = 240 * 16;
            } else {
                startAngle = 90 * 16;
                sweepAngle = -360 * 16 * qBound(0.0, data.progress, 1.0);
            }
            painter->drawPie(target, startAngle, sweepAngle);

            painter->restore();
        }

        painter->restore();
    }

    void TabBarPrivate::removeTarget(QObject * target)
    {
        while (true) {
            int index = tabBar->indexOf(target);
            if (index < 0) { // none left to remove? bail
                break;
            }
            tabBar->removeTab(index);
        }
    }

    int TabBarPrivate::tabAt(const QPoint & pos) const
    {
        if (!pos.isNull()) {
            if (!targets.isEmpty()) {
                if (const TabData * data = tabData(getCurrentIndex())) {
                    if (getTabRect(data).contains(pos)) {
                        return getCurrentIndex();
                    }
                }
                for (int index = 0; index < targets.size(); ++index) {
                    const TabData & data = targets.at(index);
                    if (index != getCurrentIndex() && getTabRect(&data).contains(pos)) {
                        return index;
                    }
                }
            }
        }
        return -1;
    }

    TabData * TabBarPrivate::tabData(int index)
    {
        return (index >= 0 && index < targets.size()) ? &targets[index] : 0;
    }

    const TabData * TabBarPrivate::tabData(int index) const
    {
        return (index >= 0 && index < targets.size()) ? &targets.at(index) : 0;
    }

    const TabData * TabBarPrivate::tabDataAt(const QPoint & pos) const
    {
        return tabData(tabAt(pos));
    }

    void TabBarPrivate::targetProgressChanged(qreal progress)
    {
        if (TabData * data = tabData(tabBar->indexOf(sender()))) {
            if (data->progress != progress) {
                bool toggle = (data->progress < 0.0 && progress >= 0.0) || (data->progress >= 0.0 && progress < 0.0);
                data->progress = progress;
                if (toggle) {
                    toggleAnimationTimer();
                } else {
                    tabBar->update();
                }
            }
        }
    }

    void TabBarPrivate::targetStateChanged(PapyroTab::State state)
    {
        if (TabData * data = tabData(tabBar->indexOf(sender()))) {
            bool error = (state == PapyroTab::DownloadingErrorState ||
                          state == PapyroTab::LoadingErrorState);
            bool busy = (state == PapyroTab::DownloadingState ||
                         state == PapyroTab::LoadingState ||
                         state == PapyroTab::ProcessingState);
            bool changed = false;

            if (data->error != error) {
                data->error = error;
                changed = true;
            }

            if (data->busy != busy) {
                data->busy = busy;
                if (busy) {
                    data->time.start();
                }
                data->progress = -1.0;
                changed = true;
            }

            if (changed) {
                updateGeometries(); // will change tab text size
                toggleAnimationTimer();
            }
        }
    }

    void TabBarPrivate::targetTitleChanged(const QString & title)
    {
        updateGeometries(); // may change tab size
    }

    void TabBarPrivate::targetUrlChanged(const QUrl & url)
    {
        tabBar->update();
    }

    void TabBarPrivate::toggleAnimationTimer()
    {
        bool needed = false;
        foreach (const TabData & data, targets) {
            if (data.busy && data.progress < 0.0) {
                needed = true;
                break;
            }
        }
        if (animationTimer.isActive()) {
            if (!needed) {
                animationTimer.stop();
            }
        } else {
            if (needed) {
                animationTimer.start();
            }
        }
        tabBar->update();
    }

    void TabBarPrivate::updateGeometries()
    {
        // Start by working out the offsets for each tab, and building the offsets map
        int offset = tabMargin;
        QMutableListIterator< TabData > iter(targets);
        while (iter.hasNext()) {
            TabData & data = iter.next();
            QString title = data.error ? "Oops..." : data.target->property("title").toString().section(" - ", 0, 0);
            int spinnerRoom = spinnerSize + 2;

            // The distance between two tabs is equal to the width of the text, plus
            // the tab edge images, minus the overlap
            int room = maxTabSize - 2 * tabEdgeSize - 2 * tabPadding - spinnerRoom;
            QFontMetrics fm(tabBar->font());
            data.title = fm.elidedText(title, Qt::ElideRight, room);
            int titleSize = fm.width(data.title);
            data.size = qMax(2 * tabEdgeSize + titleSize + spinnerRoom + 2 * tabPadding, minTabSize);
            data.offset = offset;
            offset += data.size + tabSpacing;
        }
        extent = offset - tabSpacing + tabMargin + tabFading;
        tabBar->update();
    }

    void TabBarPrivate::updateHoverPos(const QPoint & pos)
    {
        hoverPos = pos;
        int index = tabAt(hoverPos);
        if (index != tabUnderMouse) {
            QToolTip::hideText();
            tabUnderMouse = index;
            tabBar->update();
        }
        int buttonUnderMouse = getTabButtonRect(index).contains(hoverPos + QPoint(0, getPosition())) ? index : -1;
        if (buttonUnderMouse != tabButtonUnderMouse) {
            tabButtonUnderMouse = buttonUnderMouse;
            tabBar->update();
        }
    }




    TabBar::TabBar(QWidget * parent, Qt::WindowFlags f)
        : QFrame(parent, f), d(new TabBarPrivate(this))
    {
        setFixedWidth(d->activeImage.width() / d->assetScale);
        setMouseTracking(true);
    }

    TabBar::~TabBar()
    {
        // Remove all targets
    }

    int TabBar::addTab(QObject * target)
    {
        static QMap< const char *, const char * > connections;
        if (connections.isEmpty()) {
            connections["progress"] = "targetProgressChanged(qreal)";
            connections["state"] = "targetStateChanged(PapyroTab::State)";
            connections["title"] = "targetTitleChanged(const QString &)";
            connections["url"] = "targetUrlChanged(const QUrl &)";
        }

        qRegisterMetaType< PapyroTab::State >("PapyroTab::State");

        // Add target to list
        TabData data = { target, QString(), -1, -1, false, false, QTime(), -1 };
        d->targets << data;

        // Connect up signals
        connect(target, SIGNAL(destroyed(QObject*)), d, SLOT(removeTarget(QObject*)));
        QMapIterator< const char *, const char * > iter(connections);
        while (iter.hasNext()) {
            iter.next();
            QMetaProperty property = target->metaObject()->property(target->metaObject()->indexOfProperty(iter.key()));
            QMetaMethod signal = property.notifySignal();
            QMetaMethod slot = d->metaObject()->method(d->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(iter.value())));
            if (signal.methodIndex() >= 0) {
                connect(target, signal, d, slot, Qt::DirectConnection);
            }
            slot.invoke(d, Qt::DirectConnection, Q_ARG(QVariant, property.read(target)));
        }

        // Update geometries
        d->updateGeometries();

        // Make sure the current index is valid
        if (d->getCurrentIndex() == -1) {
            setCurrentIndex(0);
        }

        int newIndex = d->targets.size() - 1;

        emit layoutChanged();
        emit tabAdded(newIndex);
        emit targetAdded(target);

        // Return the index of this new tab
        return newIndex;
    }

    int TabBar::count() const
    {
        return d->targets.size();
    }

    int TabBar::currentIndex() const
    {
        return d->getCurrentIndex();
    }

    bool TabBar::event(QEvent * event)
    {
        switch (event->type()) {
        case QEvent::ToolTip: {
            if (const TabData * data = d->tabDataAt(static_cast< QHelpEvent * >(event)->pos())) {
                if (!data->error) {
                    QString title(data->target->property("title").toString());
                    if (!title.isEmpty() && title != data->title) {
                        QToolTip::showText(static_cast< QHelpEvent * >(event)->globalPos(),
                                           title, this);
                    }
                }
            } else {
                event->ignore();
            }
            return true;
        }
        default:
            break;
        }

        return QFrame::event(event);
    }

    int TabBar::indexAt(const QPoint & pos) const
    {
        return d->tabAt(pos);
    }

    int TabBar::indexOf(QObject * target) const
    {
        for (int index = 0; index < d->targets.size(); ++index) {
            if (targetAt(index) == target) {
                return index;
            }
        }
        return -1;
    }

    bool TabBar::isEmpty() const
    {
        return d->targets.isEmpty();
    }

    void TabBar::leaveEvent(QEvent * event)
    {
        d->updateHoverPos(QPoint());
    }

    void TabBar::mousePressEvent(QMouseEvent * event)
    {
        d->updateHoverPos(event->pos());
        d->tabButtonPressed = d->tabButtonUnderMouse;
        update();
    }

    void TabBar::mouseMoveEvent(QMouseEvent * event)
    {
        d->updateHoverPos(event->pos());
    }

    void TabBar::mouseReleaseEvent(QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton) {
            if (d->tabButtonPressed >= 0) {
                if (d->tabButtonUnderMouse == d->tabButtonPressed) {
                    // Close the tab under the mouse
                    emit closeRequested(d->tabButtonPressed);
                }
            } else if (d->tabUnderMouse >= 0 && d->tabUnderMouse < d->targets.size()) {
                // Raise the tab under the mouse
                setCurrentIndex(d->tabUnderMouse);
            }
            d->tabButtonPressed = -1;
            d->updateHoverPos(event->pos());
            update();
        }
    }

    void TabBar::nextTab()
    {
        setCurrentIndex((d->getCurrentIndex() + 1) % d->targets.size());
    }

    void TabBar::paintEvent(QPaintEvent * event)
    {
        // Make sure the right tab is highlighted
        d->tabUnderMouse = d->tabAt(d->hoverPos);

        QImage pixmap(width() * d->assetScale, height() * d->assetScale, QImage::Format_ARGB32_Premultiplied);
        pixmap.fill(Qt::transparent);
        if (!d->targets.isEmpty()) {
            QPainter painter(&pixmap);
            painter.scale(d->assetScale, d->assetScale);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.save();
            painter.translate(0, -d->getPosition());
            // Render each tab
            for (int index = d->targets.size() - 1; index >= 0; --index) {
                if (index != d->getCurrentIndex()) {
                    // Draw tab
                    d->paintTab(&painter, index);
                }
            }
            // Draw tab
            d->paintTab(&painter, d->getCurrentIndex());
            painter.restore();

            painter.setPen(Qt::NoPen);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            // Fade In
            /*
            QRect fadeInRect(0, 0, width(), d->tabFading);
            QLinearGradient fadeIn(fadeInRect.topLeft(), fadeInRect.bottomLeft());
            fadeIn.setColorAt(0, QColor(0, 0, 0, 0));
            fadeIn.setColorAt(1, QColor(0, 0, 0, 255));
            painter.setBrush(fadeIn);
            painter.drawRect(fadeInRect);
            */
            // Fade out
            QRect fadeOutRect(0, height() - d->tabFading, width(), d->tabFading);
            QLinearGradient fadeOut(fadeOutRect.topLeft(), fadeOutRect.bottomLeft());
            fadeOut.setColorAt(0, QColor(0, 0, 0, 255));
            fadeOut.setColorAt(1, QColor(0, 0, 0, 0));
            painter.setBrush(fadeOut);
            painter.drawRect(fadeOutRect);
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.scale(1 / (qreal) d->assetScale, 1 / (qreal) d->assetScale);
        painter.drawImage(0, 0, pixmap);
    }

    void TabBar::previousTab()
    {
        setCurrentIndex((d->getCurrentIndex() + d->targets.size() - 1) % d->targets.size());
    }

    void TabBar::removeTab(int index)
    {
        if (const TabData * data = d->tabData(index)) {
            int current = d->getCurrentIndex();
            bool removedActive = (index == current);
            if (index < current) {
                previousTab();
            }
            QObject * target = data->target;
            target->disconnect(d);
            d->targets.removeAt(index);
            if (d->currentIndex >= d->targets.size()) {
                setCurrentIndex(d->getCurrentIndex());
            } else if (removedActive) {
                emit currentIndexChanged(d->getCurrentIndex());
            }
            d->updateGeometries();
            emit layoutChanged();
            emit tabRemoved(index);
            emit targetRemoved(target);
        }
    }

    void TabBar::setCurrentIndex(int index)
    {
        if (d->currentIndex != index) {
            d->currentIndex = index;
            update();
            emit currentIndexChanged(d->getCurrentIndex());
        }
    }

    QObject * TabBar::targetAt(int index) const
    {
        return (index >= 0 && index < d->targets.size()) ? d->targets.at(index).target : 0;
    }

    void TabBar::wheelEvent(QWheelEvent * event)
    {
        if (!d->wheelDelay.isActive()) {
            setCurrentIndex(qBound(0, d->getCurrentIndex() + (event->delta() > 0 ? -1 : 1), d->targets.size() - 1));
            d->wheelDelay.start();
        }
    }

} // namespace Papyro
