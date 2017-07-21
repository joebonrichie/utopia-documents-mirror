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

#include "tabbar.h"
#include "tabbar_p.h"
#include "papyrotab.h"

#include <utopia2/qt/hidpi.h>
#include <papyro/abstractbibliography.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QHelpEvent>
#include <QImage>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QPainter>
#include <QSignalMapper>
#include <QToolTip>
#include <QVariant>
#include <QWheelEvent>

#include <QDebug>

namespace Papyro
{
    TabBarPrivate::TabBarPrivate(TabBar * tabBar)
        : QObject(tabBar), tabBar(tabBar), currentIndex(-1),
          minTabSize(100), maxTabSize(200),
          tabSpacing(-16), tabPadding(4), tabFading(10), tabMargin(6),
          spinnerSize(16),
          extent(0),
          position(0),
          tabCloseButtonPressed(-1), tabCloseButtonUnderMouse(-1),
          tabStarButtonPressed(-1), tabStarButtonUnderMouse(-1),
          dpiScaling(1)
    {
        if (Utopia::isHiDPI()) {
            dpiScaling = Utopia::hiDPIScaling();
            maxTabSize *= dpiScaling;
            minTabSize *= dpiScaling;
            tabSpacing *= dpiScaling;
            tabPadding *= dpiScaling;
            tabFading *= dpiScaling;
            tabMargin *= dpiScaling;
            spinnerSize *= dpiScaling;
        }

        tabCurveSize = QSize(28, 22) * dpiScaling;

        closeButtonIcon.addPixmap(QPixmap(":/icons/tab-close.png"));
        closeButtonIcon.addPixmap(QPixmap(":/icons/tab-close-hover.png"), QIcon::Active);

        starButtonIcon.addPixmap(QPixmap(":/icons/tab-favourite.png"));
        starButtonIcon.addPixmap(QPixmap(":/icons/tab-favourite-checked.png"), QIcon::Normal, QIcon::On);

        mouse.move.tab = -1;
        mouse.move.section = -1;
        mouse.press.section = -1;

        // One pixel for the inner part of the tab, the rest for each tab edge
        tabEdgeSize = tabCurveSize.height();

        wheelDelay.setInterval(100);
        wheelDelay.setSingleShot(true);

        animationTimer.setInterval(40);
        connect(&animationTimer, SIGNAL(timeout()), tabBar, SLOT(update()));

        connect(&citationMapper, SIGNAL(mapped(QObject *)),
                this, SLOT(onCitationChanged(QObject *)));

        tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(this, SIGNAL(closeRequested(int)),
                tabBar, SIGNAL(closeRequested(int)));

        tabBar->setMinimumWidth(tabCurveSize.width() * 1.7);

        connect(tabBar, SIGNAL(layoutChanged()), this, SLOT(updateHoverPos()));

        // FIXME use minTabSize to squash tabs down a bit
    }

    TabBarPrivate::~TabBarPrivate()
    {}

    int TabBarPrivate::getCurrentIndex() const
    {
        return tabs.isEmpty() ? -1 : qBound(0, currentIndex, tabs.size() - 1);
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

    QRect TabBarPrivate::getTabCloseButtonRect(int index) const
    {
        if (const TabData * data = tabData(index)) {
            return QRect(tabLeft() + 1 + (tabCurveSize.width() - spinnerSize) / 2, data->offset + tabEdgeSize, spinnerSize, spinnerSize);
        }
        return QRect();
    }

    QRect TabBarPrivate::getTabStarButtonRect(int index) const
    {
        if (const TabData * data = tabData(index)) {
            if (data->citation) {
                return QRect(tabLeft() + 1 + (tabCurveSize.width() - spinnerSize) / 2, data->offset + data->size - tabEdgeSize - spinnerSize, spinnerSize, spinnerSize);
            }
        }
        return QRect();
    }

    int TabBarPrivate::getTabOffset(int index) const
    {
        if (const TabData * data = tabData(index)) {
            return data->offset;
        }
        return 0;
    }

    QRect TabBarPrivate::getTabRect(int index) const
    {
        return getTabRect(tabData(index));
    }

    QRect TabBarPrivate::getTabRect(const TabData * data) const
    {
        if (data) {
            return QRect(tabLeft(), data->offset - getPosition(), tabCurveSize.width(), data->size);
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
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

        const TabData & data = tabs.at(index);
        bool hoverTab = (index == mouse.move.tab);
        //bool hoverSection = (index == mouse.move.section);

        // Draw the tab's background
        QRectF tabRect(QRectF(getTabRect(index)).adjusted(0, 0, -0.5, 0));

        QPainterPath path;
        qreal slope = 0.3;
        path.moveTo(QPointF(tabRect.right(), data.offset));
        path.quadTo(QPointF(tabRect.right(), data.offset+(tabEdgeSize*slope)), QPointF(tabRect.left()+tabRect.width()/2, data.offset+(tabEdgeSize/2.0)));
        path.quadTo(QPointF(tabRect.left(), data.offset+(tabEdgeSize*(1-slope))), QPointF(tabRect.left(), data.offset+tabEdgeSize));
        path.lineTo(QPointF(tabRect.left(), data.offset+data.size-tabEdgeSize));
        path.quadTo(QPointF(tabRect.left(), data.offset+data.size-(tabEdgeSize*(1-slope))), QPointF(tabRect.left()+tabRect.width()/2, data.offset+data.size-(tabEdgeSize/2.0)));
        path.quadTo(QPointF(tabRect.right(), data.offset+data.size-(tabEdgeSize*slope)), QPointF(tabRect.right(), data.offset+data.size));
        if (index == getCurrentIndex()) {
            path.lineTo(QPointF(tabRect.right()+1, data.offset+data.size));
            path.lineTo(QPointF(tabRect.right()+1, data.offset));
        }
        painter->setBrush(Qt::white);
        painter->setPen(Qt::black);
        painter->drawPath(path);

        if (index != getCurrentIndex()) {
            painter->setPen(Qt::black);
            painter->drawLine(tabRect.topRight(), tabRect.bottomRight());
        }

        // Draw the tab's title text
        int spinnerRoom = spinnerSize + 2;
        QRect target = QRect(0, 0, data.size - 2 * tabEdgeSize - 2 * tabPadding - spinnerRoom * (data.citation ? 2 : 1), tabCurveSize.width());
        if (!target.isEmpty()) {
            painter->save();
            painter->translate(tabLeft(), data.offset + data.size - tabEdgeSize - tabPadding - (data.citation ? spinnerRoom : 0));
            painter->rotate(-90);
            QPixmap textPixmap(QSize(target.width(), target.height()) * Utopia::retinaScaling());
            textPixmap.fill(Qt::transparent);
            {
                QPainter textPainter(&textPixmap);
                textPainter.setRenderHint(QPainter::Antialiasing, true);
                textPainter.setRenderHint(QPainter::TextAntialiasing, true);
                textPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                if (data.error) {
                    textPainter.setPen(QColor(200, 0, 0));
                }
                textPainter.scale(Utopia::retinaScaling(), Utopia::retinaScaling());
                target.setSize(target.size());
                textPainter.drawText(target, Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignLeft, data.title);
            }
            painter->scale(1 / Utopia::retinaScaling(), 1 / Utopia::retinaScaling());
            painter->drawPixmap(0, 0, textPixmap);
            painter->restore();
        }

        // Draw the tab's actions
        bool isCloseActionVisible = (!data.busy || tabCloseButtonUnderMouse == index || tabCloseButtonPressed == index);
        if (isCloseActionVisible) {
            painter->save();
            painter->setOpacity(0.6);
            target = getTabCloseButtonRect(index);
            QIcon::Mode mode(QIcon::Normal);
            if (tabCloseButtonUnderMouse == index || tabCloseButtonPressed == index) {
                mode = QIcon::Active;
                if (tabCloseButtonUnderMouse == tabCloseButtonPressed) {
                    painter->setOpacity(0.8);
                }
            }
            painter->drawPixmap(target, closeButtonIcon.pixmap(target.size() * Utopia::retinaScaling(), mode, QIcon::Off));
            painter->restore();
        }
        if (true) {
            painter->save();
            painter->setOpacity(0.8);
            target = getTabStarButtonRect(index);
            QIcon::Mode mode(QIcon::Normal);
            if (tabStarButtonUnderMouse == index && tabStarButtonPressed == index) {
                painter->setOpacity(1.0);
            }
            QIcon::State state(QIcon::Off);
            if (data.starred) {
                state = QIcon::On;
            }
            painter->drawPixmap(target, starButtonIcon.pixmap(target.size() * Utopia::retinaScaling(), mode, state));
            painter->restore();
        }

        // The rectangle into which we place the spinner / close button
        target = getTabCloseButtonRect(index);
        //bool hoverClose = target.contains(hoverPos + QPoint(0, getPosition()));
        //bool pressClose = tabCloseButtonPressed == index;

        if (!isCloseActionVisible) {
            painter->save();

            // Draw the tab's progress spinner

            painter->setOpacity(0.6);
            QPixmap spinner(target.size() * Utopia::retinaScaling());
            spinner.fill(Qt::transparent);
            {
                QPainter spinnerPainter(&spinner);
                spinnerPainter.setRenderHint(QPainter::Antialiasing, true);

                // Clip an internal circle
                QPainterPath clip;
                QRect rect(spinner.rect());
                clip.addRect(rect);
                qreal width = rect.width() / 4.0;
                clip.addEllipse(rect.adjusted(width, width, -width, -width));
                spinnerPainter.setClipPath(clip);
                rect.adjust(Utopia::retinaScaling(), Utopia::retinaScaling(), -Utopia::retinaScaling(), -Utopia::retinaScaling());

                spinnerPainter.setPen(Qt::NoPen);
                if (data.progress >= 0.0) {
                    spinnerPainter.setBrush(QColor(0, 0, 0, 40));
                    spinnerPainter.drawEllipse(rect);
                }
                spinnerPainter.setBrush(Qt::black);
                int startAngle, sweepAngle;
                if (data.progress < 0.0) {
                    startAngle = -(data.time.elapsed() * 7 % (360 * 16));
                    sweepAngle = 240 * 16;
                } else {
                    startAngle = 90 * 16;
                    sweepAngle = -360 * 16 * qBound(0.0, data.progress, 1.0);
                }
                spinnerPainter.drawPie(rect, startAngle, sweepAngle);
            }
            painter->drawPixmap(target, spinner);

            painter->restore();
        }

        // Draw highlight
        if (data.known) {
            painter->save();
            QRectF bounds(tabRect.adjusted(0, 0, -tabRect.width() * 5 / 6, 0));
            QLinearGradient gradient(bounds.topLeft(), bounds.topRight());
            QString color("66CC00");
            gradient.setColorAt(0, QColor("#ff"+color));
            gradient.setColorAt(0.6, QColor("#ff"+color));
            gradient.setColorAt(1, QColor("#00"+color));
            painter->setBrush(gradient);
            painter->setBrush(QColor("#"+color));
            painter->setPen(Qt::NoPen);
            painter->setCompositionMode(QPainter::CompositionMode_Multiply);
            QPainterPath stamp;
            const qreal thickness = 4 * dpiScaling;
            stamp.moveTo(QPointF(tabRect.right(), data.offset));
            stamp.quadTo(QPointF(tabRect.right(), data.offset+(tabEdgeSize*slope)), QPointF(tabRect.left()+tabRect.width()/2, data.offset+(tabEdgeSize/2.0)));
            stamp.quadTo(QPointF(tabRect.left()+thickness, data.offset+(tabEdgeSize*(1-slope))), QPointF(tabRect.left()+thickness, data.offset+tabEdgeSize+thickness));
            stamp.lineTo(QPointF(tabRect.left()+thickness, data.offset+data.size-tabEdgeSize-thickness));
            stamp.quadTo(QPointF(tabRect.left()+thickness, data.offset+data.size-(tabEdgeSize*(1-slope))), QPointF(tabRect.left()+tabRect.width()/2, data.offset+data.size-(tabEdgeSize/2.0)));
            stamp.quadTo(QPointF(tabRect.right(), data.offset+data.size-(tabEdgeSize*slope)), QPointF(tabRect.right(), data.offset+data.size));
            stamp.lineTo(QPointF(tabRect.right()+1, data.offset+data.size));
            stamp.lineTo(QPointF(tabRect.right()+1, data.offset));
            painter->drawPath(path.subtracted(stamp));
            painter->restore();
        }

        // Draw lowlight
        {
            QColor grey(QColor("#888888"));
            painter->save();
            painter->setCompositionMode(QPainter::CompositionMode_Multiply);
            painter->setOpacity(index == getCurrentIndex() ? 0.0 : hoverTab ? 0.5 : 0.7);
            painter->setPen(grey);
            painter->setBrush(grey);
            painter->drawPath(path);
            painter->restore();
        }

        painter->restore();
    }

    void TabBarPrivate::onCitationChanged(QObject * obj)
    {
        if (PapyroTab * tab = qobject_cast< PapyroTab * >(obj)) {
            if (TabData * data = tabData(tabBar->indexOf(tab))) {
                updateState(data);
                updateGeometries();
                updateHoverPos();
            }
        }
    }

    void TabBarPrivate::onTabCitationChanged()
    {
        // A tab's citation could change because it has been resolved
        if (PapyroTab * tab = qobject_cast< Papyro::PapyroTab * >(sender())) {
            if (TabData * data = tabData(tabBar->indexOf(tab))) {
                // If an old one is present, disconnect it
                if (data->citation) {
                    data->citation->disconnect(this);
                    data->citation->disconnect(&citationMapper);
                }
                // Connect the new one, if available
                data->citation = tab->citation();
                if (data->citation) {
                    citationMapper.setMapping(data->citation.get(), (QObject *) tab);
                    connect(data->citation.get(), SIGNAL(changed()),
                            &citationMapper, SLOT(map()));
                }
                updateState(data);
                updateGeometries();
                updateHoverPos();
            }
        }
    }

    int TabBarPrivate::tabAt(const QPoint & pos) const
    {
        if (!pos.isNull()) {
            if (!tabs.isEmpty()) {
                // First check the current tab
                int current = getCurrentIndex();
                if (const TabData * data = tabData(current)) {
                    if (getTabRect(data).contains(pos)) {
                        return current;
                    }
                }
                for (int index = 0; index < tabs.size(); ++index) {
                    const TabData & data = tabs.at(index);
                    if (index != getCurrentIndex() && getTabRect(&data).contains(pos)) {
                        return index;
                    }
                }
            }
        }
        return -1;
    }

    void TabBarPrivate::tabCloseRequested()
    {
        emit closeRequested(tabBar->indexOf(qobject_cast< Papyro::PapyroTab * >(sender())));
    }

    TabData * TabBarPrivate::tabData(int index)
    {
        return (index >= 0 && index < tabs.size()) ? &tabs[index] : 0;
    }

    const TabData * TabBarPrivate::tabData(int index) const
    {
        return (index >= 0 && index < tabs.size()) ? &tabs.at(index) : 0;
    }

    const TabData * TabBarPrivate::tabDataAt(const QPoint & pos) const
    {
        return tabData(tabAt(pos));
    }

    void TabBarPrivate::tabDestroyed(QObject * obj)
    {
        while (true) {
            int index = 0;
            foreach (const TabData & data, tabs) {
                if (data.tab == obj) {
                    tabBar->removeTab(index);
                    break;
                }
                ++index;
            }
            if (index >= tabs.size()) { // none left to remove? bail
                break;
            }
        }
    }

    int TabBarPrivate::tabLeft() const
    {
        return tabBar->width() - tabCurveSize.width();
    }

    void TabBarPrivate::tabProgressChanged(qreal progress)
    {
        if (TabData * data = tabData(tabBar->indexOf(qobject_cast< Papyro::PapyroTab * >(sender())))) {
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

    void TabBarPrivate::tabStateChanged(PapyroTab::State state)
    {
        if (TabData * data = tabData(tabBar->indexOf(qobject_cast< Papyro::PapyroTab * >(sender())))) {
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
                updateHoverPos();
            }
        }
    }

    void TabBarPrivate::tabTitleChanged(const QString & title)
    {
        updateGeometries(); // may change tab size
        updateHoverPos();
    }

    void TabBarPrivate::tabUrlChanged(const QUrl & url)
    {
        tabBar->update();
    }

    void TabBarPrivate::toggleAnimationTimer()
    {
        bool needed = false;
        foreach (const TabData & data, tabs) {
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
        QMutableListIterator< TabData > iter(tabs);
        while (iter.hasNext()) {
            TabData & data = iter.next();
            QString title = data.error ? "Oops..." : data.tab->property("title").toString().section(" - ", 0, 0);
            int spinnerRoom = spinnerSize + 2;

            // The distance between two tabs is equal to the width of the text, plus
            // the tab edge images, minus the overlap
            int room = maxTabSize - 2 * tabEdgeSize - 2 * tabPadding - spinnerRoom * (data.citation ? 2 : 1);
            QFontMetrics fm(tabBar->font());
            data.title = fm.elidedText(title, Qt::ElideRight, room);
            int titleSize = fm.width(data.title);
            data.size = qMax(2 * tabEdgeSize + titleSize + spinnerRoom * (data.citation ? 2 : 1) + 2 * tabPadding, minTabSize);
            data.offset = offset;
            offset += data.size + tabSpacing;
        }
        extent = offset - tabSpacing + tabMargin + tabFading;
        tabBar->update();
    }

    void TabBarPrivate::updateHoverPos()
    {
        hoverPos = tabBar->mapFromGlobal(QCursor::pos());

        // Actual visible tabs
        int index = tabAt(hoverPos);
        if (index != mouse.move.tab) {
            QToolTip::hideText();
            mouse.move.tab = index;
            tabBar->update();
        }

        int closeButtonUnderMouse = getTabCloseButtonRect(index).contains(hoverPos + QPoint(0, getPosition())) ? index : -1;
        if (closeButtonUnderMouse != tabCloseButtonUnderMouse) {
            tabCloseButtonUnderMouse = closeButtonUnderMouse;
            tabBar->update();
        }

        int starButtonUnderMouse = getTabStarButtonRect(index).contains(hoverPos + QPoint(0, getPosition())) ? index : -1;
        if (starButtonUnderMouse != tabStarButtonUnderMouse) {
            tabStarButtonUnderMouse = starButtonUnderMouse;
            tabBar->update();
        }

        // Tab area
        if (mouse.press.section == -1) {
            int index = tabAt(hoverPos);
            if (index != mouse.move.section) {
                mouse.move.section = index;
                tabBar->update();
            }
        }
        tabBar->update();
    }

    void TabBarPrivate::updateState(TabData * data)
    {
        bool known = data->citation && data->citation->field(Athenaeum::Citation::KnownRole).toBool();
        bool starred = data->citation && (data->citation->field(Athenaeum::Citation::FlagsRole).value< Athenaeum::Citation::Flags >() & Athenaeum::Citation::StarredFlag);
        data->known = known;
        data->starred = known && starred;
        tabBar->update();
    }




    TabBar::TabBar(QWidget * parent, Qt::WindowFlags f)
        : QFrame(parent, f), d(new TabBarPrivate(this))
    {
        setMouseTracking(true);
    }

    TabBar::~TabBar()
    {
        // Remove all tabs
    }

    int TabBar::addTab(PapyroTab * tab)
    {
        static QMap< const char *, const char * > connections;
        if (connections.isEmpty()) {
            connections["progress"] = "tabProgressChanged(qreal)";
            connections["state"] = "tabStateChanged(PapyroTab::State)";
            connections["title"] = "tabTitleChanged(const QString &)";
            connections["url"] = "tabUrlChanged(const QUrl &)";
        }

        qRegisterMetaType< PapyroTab::State >("PapyroTab::State");

        TabData data = { tab, tab->citation(), QString(), -1, -1, false, false, QTime(), -1, false, false };
        // Add tab to list
        d->tabs << data;

        // Connect up signals
        connect(tab, SIGNAL(citationChanged()),
                d, SLOT(onTabCitationChanged()));
        if (data.citation) {
            d->citationMapper.setMapping(data.citation.get(), (QObject *) tab);
            connect(data.citation.get(), SIGNAL(changed()),
                    &d->citationMapper, SLOT(map()));
            d->updateState(&data);
        }

        connect(tab, SIGNAL(destroyed(QObject*)), d, SLOT(tabDestroyed(QObject*)));
        connect(tab, SIGNAL(closeRequested()), d, SLOT(tabCloseRequested()));
        QMapIterator< const char *, const char * > iter(connections);
        while (iter.hasNext()) {
            iter.next();
            QMetaProperty property = tab->metaObject()->property(tab->metaObject()->indexOfProperty(iter.key()));
            QMetaMethod signal = property.notifySignal();
            QMetaMethod slot = d->metaObject()->method(d->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(iter.value())));
            if (signal.methodIndex() >= 0) {
                connect(tab, signal, d, slot, Qt::DirectConnection);
            }
            //slot.invoke(d, Qt::DirectConnection, Q_ARG(QVariant, property.read(tab)));
        }

        // Update geometries
        d->updateGeometries();

        // Make sure the current index is valid
        if (d->getCurrentIndex() == -1) {
            setCurrentIndex(0);
        }

        int newIndex = d->tabs.size() - 1;

        emit layoutChanged();
        emit tabAdded(newIndex);
        emit tabAdded(tab);

        // Return the index of this new tab
        return newIndex;
    }

    int TabBar::count() const
    {
        return d->tabs.size();
    }

    int TabBar::currentIndex() const
    {
        return d->getCurrentIndex();
    }

    void TabBar::enterEvent(QEvent * event)
    {
        d->updateHoverPos();
    }

    bool TabBar::event(QEvent * event)
    {
        switch (event->type()) {
        case QEvent::ToolTip: {
            QPoint pos(static_cast< QHelpEvent * >(event)->pos());
            QPoint globalPos(static_cast< QHelpEvent * >(event)->globalPos());
            const TabData * data = d->tabDataAt(pos);
            if (d->tabCloseButtonUnderMouse >= 0) {
                QToolTip::showText(globalPos, "Close Tab", this);
            } else if (d->tabStarButtonUnderMouse >= 0) {
                QString toolTip(data->starred ? "Unstar this Article" : "Star this Article");
                if (!data->known && !data->starred) {
                    toolTip += " (and Save to Library)";
                }
                QToolTip::showText(globalPos, toolTip, this);
            } else if (data) {
                if (!data->error) {
                    QString title(data->tab->property("title").toString());
                    if (!title.isEmpty() && title != data->title) {
                        QToolTip::showText(globalPos, title, this);
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

    int TabBar::indexOf(PapyroTab * tab) const
    {
        for (int index = 0; index < d->tabs.size(); ++index) {
            if (tabAt(index) == tab) {
                return index;
            }
        }
        return -1;
    }

    bool TabBar::isEmpty() const
    {
        return d->tabs.isEmpty();
    }

    void TabBar::leaveEvent(QEvent * event)
    {
        d->updateHoverPos();
    }

    void TabBar::mousePressEvent(QMouseEvent * event)
    {
        d->updateHoverPos();

        if (event->button() == Qt::LeftButton) {
            d->mouse.press.section = d->mouse.move.section;
            d->tabCloseButtonPressed = d->tabCloseButtonUnderMouse;
            d->tabStarButtonPressed = d->tabStarButtonUnderMouse;
            update();
        }
    }

    void TabBar::mouseMoveEvent(QMouseEvent * event)
    {
        d->updateHoverPos();
    }

    void TabBar::mouseReleaseEvent(QMouseEvent * event)
    {
        d->updateHoverPos();

        if (event->button() == Qt::LeftButton) {
            if (d->tabCloseButtonPressed == d->mouse.move.section) {
                if (/* TabData * data = */ d->tabData(d->mouse.press.section)) {
                    emit closeRequested(d->mouse.press.section);
                }
            } else if (d->tabStarButtonPressed == d->mouse.move.section) {
                if (TabData * data = d->tabData(d->mouse.press.section)) {
                    if (data->citation) {
                        if (data->citation->isStarred()) {
                            data->tab->unstar();
                        } else {
                            data->tab->star();
                        }
                    }
                }
            } else if (d->mouse.move.tab >= 0 && d->mouse.move.tab < d->tabs.size()) {
                // Raise the tab under the mouse
                setCurrentIndex(d->mouse.move.tab);
            }
            d->tabCloseButtonPressed = -1;
            d->tabStarButtonPressed = -1;
            d->mouse.press.section = -1;
            update();
        }
    }

    void TabBar::nextTab()
    {
        setCurrentIndex((d->getCurrentIndex() + 1) % d->tabs.size());
    }

    void TabBar::paintEvent(QPaintEvent * event)
    {
        // Make sure the right tab is highlighted
        d->mouse.move.tab = d->tabAt(d->hoverPos);

        QImage pixmap(size() * Utopia::retinaScaling(), QImage::Format_ARGB32_Premultiplied);
        pixmap.fill(Qt::transparent);
        if (!d->tabs.isEmpty()) {
            QPainter painter(&pixmap);
            painter.scale(Utopia::retinaScaling(), Utopia::retinaScaling());
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.save();
            painter.translate(0, -d->getPosition());
            // Render each tab
            for (int index = d->tabs.size() - 1; index >= 0; --index) {
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

        painter.scale(1 / Utopia::retinaScaling(), 1 / Utopia::retinaScaling());
        painter.drawImage(0, 0, pixmap);
    }

    void TabBar::previousTab()
    {
        setCurrentIndex((d->getCurrentIndex() + d->tabs.size() - 1) % d->tabs.size());
    }

    void TabBar::removeTab(int index)
    {
        if (const TabData * data = d->tabData(index)) {
            int current = d->getCurrentIndex();
            bool removedActive = (index == current);
            if (index < current) {
                previousTab();
            }
            PapyroTab * tab = data->tab;
            if (tab) {
                tab->disconnect(d);
                if (data->citation) {
                    data->citation->disconnect(&d->citationMapper);
                }
            }
            d->tabs.removeAt(index);
            if (d->currentIndex >= d->tabs.size()) {
                setCurrentIndex(d->getCurrentIndex());
            } else if (removedActive) {
                emit currentIndexChanged(d->getCurrentIndex());
            }
            d->updateGeometries();
            emit layoutChanged();
            emit tabRemoved(index);
            if (tab) { emit tabRemoved(tab); }
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

    PapyroTab * TabBar::tabAt(int index) const
    {
        return (index >= 0 && index < d->tabs.size()) ? d->tabs.at(index).tab : 0;
    }

    void TabBar::wheelEvent(QWheelEvent * event)
    {
        if (!d->wheelDelay.isActive() && event->delta() != 0) {
            setCurrentIndex(qBound(0, d->getCurrentIndex() + (event->delta() > 0 ? -1 : 1), d->tabs.size() - 1));
            d->wheelDelay.start();
        }
    }

} // namespace Papyro
