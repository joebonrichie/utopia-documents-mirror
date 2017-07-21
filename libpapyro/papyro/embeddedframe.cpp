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

#include <papyro/embeddedpane.h>
#include <papyro/embeddedframe.h>
#include <papyro/embeddedframe_p.h>
#include <papyro/embeddedpanefactory.h>
#include <papyro/playercontrols.h>

#include <string>

#include <QMap>
#include <QMenu>
#include <QPainter>
#include <QSignalMapper>
#include <QStackedLayout>
#include <QTimer>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QtDebug>

namespace Papyro
{

    class EmbeddedFramePrivate
    {
    public:
        Spine::AnnotationHandle annotation;
        QRectF bounds;
        bool hover;
        bool awaitingUnhover;
        QTimer unhoverTimer;
        bool embeddable;

        // Visualisation Widgets
        QList< QString > panes;
        QList< QWidget * > widgets;
        QStackedLayout * stack;
        QSignalMapper * playMapper;
        QSignalMapper * launchMapper;
        PlayerControls * controls;



        // HACK FIXME
        QToolButton * graphButton;
        bool table;
    };

    EmbeddedFrame::EmbeddedFrame(Spine::AnnotationHandle annotation, const QRectF & bounds, QWidget * parent)
        : QWidget(parent), d(new EmbeddedFramePrivate)
    {
        d->annotation = annotation;
        d->bounds = bounds;
        d->hover = false;
        d->awaitingUnhover = false;
        d->embeddable = true;
        d->unhoverTimer.setInterval(800);
        d->unhoverTimer.setSingleShot(true);
        connect(&d->unhoverTimer, SIGNAL(timeout()), this, SLOT(hideControls()));

        QVBoxLayout * vLayout = new QVBoxLayout(this);
        vLayout->setSpacing(1);
        vLayout->setContentsMargins(0, 0, 0, 0);
        d->stack = new QStackedLayout();
        vLayout->addLayout(d->stack);
        QHBoxLayout * hLayout = new QHBoxLayout();
        hLayout->setSpacing(0);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->addStretch(1);
        d->controls = new PlayerControls(this);
        hLayout->addWidget(d->controls);
        connect(d->controls, SIGNAL(pauseClicked()), this, SLOT(onCloseClicked()));
        connect(d->controls, SIGNAL(launchClicked()), this, SLOT(onLaunchClicked()));
        connect(d->controls, SIGNAL(magnifyClicked()), this, SLOT(onMagnifyClicked()));
        connect(d->controls, SIGNAL(playClicked()), this, SLOT(onPlayClicked()));
        vLayout->addLayout(hLayout);

        setContentsMargins(1, 1, 1, 0);
        setMouseTracking(true);

        d->playMapper = new QSignalMapper(this);
        QObject::connect(d->playMapper, SIGNAL(mapped(int)), d->stack, SLOT(setCurrentIndex(int)));
        d->launchMapper = new QSignalMapper(this);
        QObject::connect(d->launchMapper, SIGNAL(mapped(int)), this, SLOT(launchPane(int)));
    }

    EmbeddedFrame::~EmbeddedFrame()
    {
        delete d;
    }

    Spine::AnnotationHandle EmbeddedFrame::annotation() const
    {
        return d->annotation;
    }

    const QRectF & EmbeddedFrame::bounds() const
    {
        return d->bounds;
    }

    QSize EmbeddedFrame::controlSize() const
    {
        return d->controls->size();
    }

    void EmbeddedFrame::enterEvent(QEvent * event)
    {
        d->awaitingUnhover = false;
        d->hover = true;
        remask();
        setCursor(Qt::ArrowCursor);
        update();
    }

    void EmbeddedFrame::hideControls()
    {
        if (d->awaitingUnhover)
        {
            d->unhoverTimer.stop();
            d->awaitingUnhover = false;
            d->hover = false;
            remask();
            update();
        }
    }

    void EmbeddedFrame::launchPane(int idx)
    {
        if (idx >= 0 && idx < d->widgets.size())
        {
            QWidget * w = d->widgets.at(idx);
            w->resize(w->size() + QSize(1, 1));
            w->show();
            w->raise();
        }
    }

    void EmbeddedFrame::leaveEvent(QEvent * event)
    {
        d->awaitingUnhover = true;
        d->unhoverTimer.start();
    }

    void EmbeddedFrame::onCloseClicked()
    {
        d->stack->setCurrentIndex(d->stack->count() - 1);
        if (d->graphButton) { d->graphButton->setEnabled(false); }
    }

    void EmbeddedFrame::onMagnifyClicked()
    {
    }

    void EmbeddedFrame::onLaunchClicked()
    {
        if (d->panes.size() == 1)
        {
            launchPane(0);
        }
        else
        {
            QMenu * menu = new QMenu(this);
            int idx = 0;
            QListIterator< QString > i(d->panes);
            while (i.hasNext())
            {
                d->launchMapper->setMapping(menu->addAction(i.next(), d->launchMapper, SLOT(map())), idx);
                ++idx;
            }
            menu->exec(d->controls->launchPos());
        }
    }

    void EmbeddedFrame::onPlayClicked()
    {
        if (d->panes.size() == 1)
        {
            d->stack->setCurrentIndex(0);
            if (d->graphButton) { d->graphButton->setEnabled(true); }
        }
        else
        {
            QMenu * menu = new QMenu(this);
            int idx = 0;
            QListIterator< QString > i(d->panes);
            while (i.hasNext())
            {
                d->playMapper->setMapping(menu->addAction(i.next(), d->playMapper, SLOT(map())), idx);
                ++idx;
            }
            menu->exec(d->controls->playPos());
        }
    }

    void EmbeddedFrame::paintEvent(QPaintEvent * event)
    {
        if (d->hover)
        {
            QPainter p(this);
            p.setBrush(Qt::NoBrush);
            p.setPen(QColor(50, 50, 50));
            p.drawRect(rect().adjusted(0, 0, -1, -1 - d->controls->height()));
        }
    }

    void EmbeddedFrame::mouseReleaseEvent(QMouseEvent * event)
    {
        if (!d->embeddable) {
            launchPane(0);
        }
        event->ignore();
    }

    void EmbeddedFrame::remask()
    {
        QRegion mask;
        if (d->embeddable && d->hover) {
            mask += d->controls->geometry();
        }
        mask += QRect(0, 0, width(), height() - d->controls->height());
        setMask(mask);
    }

    void EmbeddedFrame::resizeEvent(QResizeEvent * event)
    {
        remask();
    }

    void EmbeddedFrame::showControls(int msecs)
    {
        d->unhoverTimer.setInterval(msecs);
        // Prod to start
        enterEvent(0);
        // Prod to stop
        leaveEvent(0);
    }

    void EmbeddedFrame::onGraphClicked()
    {
        if (d->table)
        {
            d->graphButton->setStyleSheet(  "QToolButton {"
                                            "    border-image: url(:/icons/graph.png);"
                                            " }"
                                            ""
                                            "QToolButton:pressed {"
                                            "    border-image: url(:/icons/graph-pressed.png);"
                                            " }"
                );
        }
        else
        {
            d->graphButton->setStyleSheet(  "QToolButton {"
                                            "    border-image: url(:/icons/table.png);"
                                            " }"
                                            ""
                                            "QToolButton:pressed {"
                                            "    border-image: url(:/icons/table-pressed.png);"
                                            " }"
                );
        }

        d->table = !d->table;

        Q_EMIT flip();
    }

    QWidget * EmbeddedFrame::probe(Spine::AnnotationHandle annotation)
    {
        // Make sure there's a static cache of pane factories
        static std::set< EmbeddedPaneFactory * > factories;
        if (factories.size() == 0)
        {
            factories = Utopia::instantiateAllExtensions< EmbeddedPaneFactory >();
        }

        BOOST_FOREACH(EmbeddedPaneFactory * factory, factories)
        {
            QWidget * pane = factory->create(annotation, 0);
            if (pane) { return pane; }
        }

        return 0;
    }

    EmbeddedFrame * EmbeddedFrame::probe(Spine::AnnotationHandle annotation, const QRectF & bounds, QWidget * parent)
    {
        EmbeddedFrame * frame = 0;

        // Make sure there's a static cache of pane factories
        static std::set< EmbeddedPaneFactory * > factories;
        if (factories.size() == 0)
        {
            factories = Utopia::instantiateAllExtensions< EmbeddedPaneFactory >();
        }

        QMap< QString, QPair< QWidget *, QWidget * > > panes;
        BOOST_FOREACH(EmbeddedPaneFactory * factory, factories)
        {
            QWidget * pane = factory->create(annotation, parent);
            if (pane)
            {
                QWidget * pane2 = factory->create(annotation, 0);
                pane2->hide();
                panes[factory->title()] = QPair< QWidget *, QWidget * >(pane, pane2);
            }
        }

        if (panes.size() > 0)
        {
            frame = new EmbeddedFrame(annotation, bounds, parent);
            EmbeddedPane * pane = 0;

            int idx = 0;
            QMapIterator< QString, QPair< QWidget *, QWidget * > > i(panes);
            while (i.hasNext())
            {
                i.next();
                frame->d->stack->addWidget(i.value().first);
                frame->d->widgets.append(i.value().second);
                frame->d->panes.append(i.key());
                ++idx;

                pane = qobject_cast< EmbeddedPane * >(i.value().first);

                // HACK to stop multiple panes
                break;
            }

            std::string embeddable(annotation->getFirstProperty("property:embeddable"));

            if (pane && embeddable == "no") {
                pane->setInteractionFlags(pane->interactionFlags() & ~EmbeddedPane::Embeddable);
                frame->setCursor(Qt::PointingHandCursor);
                frame->d->embeddable = false;
            }

            QWidget * spacer = new QWidget(frame);
            spacer->setEnabled(false);
            frame->d->stack->addWidget(spacer);
            frame->d->stack->setCurrentIndex(idx);

            // FIXME HACK Add button to graphs
            std::string concept(annotation->getFirstProperty("concept"));
            // FIXME remove the shorter namespace
            if (concept == "http://utopia.cs.man.ac.uk/utopia/annotation#graph" ||
                concept == "http://utopia.cs.manchester.ac.uk/utopia/annotation#graph")
            {
                frame->d->graphButton = new QToolButton();
                frame->d->graphButton->setStyleSheet(  "QToolButton {"
                                                       "    border-image: url(:/icons/graph.png);"
                                                       " }"
                                                       ""
                                                       "QToolButton:pressed {"
                                                       "    border-image: url(:/icons/graph-pressed.png);"
                                                       " }"
                    );
                connect(frame->d->graphButton, SIGNAL(clicked()), frame, SLOT(onGraphClicked()));
                connect(frame, SIGNAL(flip()), i.value().first, SLOT(flip()));
                frame->d->controls->addControl(frame->d->graphButton);
                frame->d->table = false;
                frame->d->graphButton->setEnabled(false);
            } else {
                frame->d->graphButton = 0;
                frame->d->table = false;
            }
        }

        return frame;
    }

}
