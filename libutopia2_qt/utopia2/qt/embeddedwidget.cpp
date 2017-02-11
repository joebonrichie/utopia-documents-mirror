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

#include <boost/foreach.hpp>
#include <utopia2/qt/abstractwidget.h>
#include <utopia2/qt/embeddedwidget.h>
#include <utopia2/node.h>
#include <utopia2/extension.h>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>


#include <iostream>
#include <QtDebug>

namespace Utopia
{

    HeadUpDisplay::HeadUpDisplay(QWidget * parent, Qt::WindowFlags f)
        : QWidget(parent, f), _visible(false)
    {
        this->setMouseTracking(true);
    }

    HeadUpDisplay::~HeadUpDisplay()
    {}

    bool HeadUpDisplay::eventFilter(QObject * obj, QEvent * event)
    {
        if (obj == this->parent() && event->type() == QEvent::Resize)
        {
            QWidget * parent = qobject_cast< QWidget * >(obj);
            this->setGeometry(QRect(parent->rect().bottomRight() - QPoint(11, 11), QSize(12, 12)));
        }

        // standard event processing
        return QWidget::eventFilter(obj, event);
    }

    void HeadUpDisplay::leaveEvent(QEvent * /* event */)
    {
        emit hover(false);
    }

    void HeadUpDisplay::enterEvent(QEvent * /* event */)
    {
        emit hover(true);
    }

    void HeadUpDisplay::mousePressEvent(QMouseEvent * event)
    {
        this->_visible = !this->_visible;
        emit visibilityToggled(this->_visible);
    }

    void HeadUpDisplay::paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(0.5, 0.5);
        painter.fillRect(this->rect(), QColor(180, 180, 180));
        QRect rect = this->rect().adjusted(3, 3, -3, -3);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QBrush(Qt::white), 2));
        painter.drawLine(rect.topLeft(), rect.bottomRight() - QPoint(1, 1));
        if (this->_visible)
        {
            painter.drawLine(rect.bottomLeft(), rect.bottomRight());
            painter.drawLine(rect.bottomRight(), rect.topRight());
        }
        else
        {
            painter.drawLine(rect.bottomLeft(), rect.topLeft());
            painter.drawLine(rect.topLeft(), rect.topRight());
        }
        painter.end();

        this->QWidget::paintEvent(event);
    }




    EmbeddedWidget::EmbeddedWidget(QWidget * parent, Qt::WindowFlags f)
        : QFrame(parent, f)
    {
        // Initialise
        this->initialise(0);
    }

    EmbeddedWidget::EmbeddedWidget(Node * model, QWidget * parent, Qt::WindowFlags f)
        : QFrame(parent, f)
    {
        this->initialise(model);
    }

    EmbeddedWidget::~EmbeddedWidget()
    {}

    /**
     *  \brief Initialise widget UI.
     */
    void EmbeddedWidget::initialise(Node * model)
    {
        this->_abstractWidget = 0;
        this->setFrameStyle(QFrame::Plain | QFrame::Box);
//      this->setFrameStyle(QFrame::NoFrame);
        QPalette p = this->palette();
        p.setColor(QPalette::WindowText, QColor(200, 200, 200, 0));
        p.setColor(QPalette::Window, QColor(240, 240, 240));
        this->setPalette(p);
        this->_visible = false;

        this->_model = 0;

        this->_hud = new HeadUpDisplay(this);
        this->installEventFilter(this->_hud);
        this->_hud->raise();
        connect(this->_hud, SIGNAL(visibilityToggled(bool)), this, SLOT(showData(bool)));
        connect(this->_hud, SIGNAL(hover(bool)), this, SLOT(hover(bool)));

        // Set model if need be
        if (model)
        {
            this->load(model);
        }
    }

    void EmbeddedWidget::load(Node * model)
    {
        // Remove old widget if present
        if (this->_abstractWidget)
        {
            delete this->_abstractWidget;
            this->_abstractWidget = 0;
        }

        this->_model = model;

        std::cout << "============= " << registeredExtensionNames< AbstractWidget >().size() << std::endl;

        BOOST_FOREACH(std::string name, registeredExtensionNames< AbstractWidget >())
        {
            std::cout << "EmbeddedWidget::load" << "trying" << name << std::endl;
            AbstractWidget * abstractWidget = instantiateExtension< AbstractWidget >(name);
            if (abstractWidget)
            {
                qDebug() << "EmbeddedWidget::load" << "  found" << abstractWidget;
                if (abstractWidget->supports(model))
                {
                    qDebug() << "EmbeddedWidget::load" << "    supports" << abstractWidget;
                    this->_abstractWidget = abstractWidget;
                    abstractWidget->asWidget()->hide();

                    // Set up user interface
                    QHBoxLayout * layout = new QHBoxLayout(this);
                    layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    layout->setContentsMargins(0, 0, 0, 0);
                    layout->setSpacing(0);
                    layout->addWidget(abstractWidget->asWidget());
                    abstractWidget->asWidget()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
//                                      abstractWidget->asWidget()->show();
                    abstractWidget->asWidget()->hide();

                    qDebug() << "showing abstractWidget in geometry" << abstractWidget->asWidget()->geometry();

//                                      abstractWidget->load(model);
                    qDebug() << "-----";
                    break;
                }
                else
                {
                    delete abstractWidget;
                }
            }
        }

        this->_hud->raise();
    }

    void EmbeddedWidget::hover(bool hover)
    {
        if (hover)
        {
            QPalette p = this->palette();
            p.setColor(QPalette::WindowText, QColor(200, 200, 200, 255));
            this->setPalette(p);
        }
        else
        {
            QPalette p = this->palette();
            p.setColor(QPalette::WindowText, QColor(200, 200, 200, 0));
            this->setPalette(p);
        }
    }

    void EmbeddedWidget::showData(bool visible)
    {
        this->_abstractWidget->asWidget()->setVisible(visible);

        if (this->_model && visible)
        {
            this->_abstractWidget->load(this->_model);
            this->_model = 0;
        }
    }

} // namespace FINDOMATIC
