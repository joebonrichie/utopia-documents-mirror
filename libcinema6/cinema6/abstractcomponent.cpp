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

#include <cinema6/abstractcomponent.h>
#include <cinema6/alignmentview.h>
#include <cinema6/renderevent.h>

#include <QEvent>
#include <QMouseEvent>

namespace CINEMA6
{

    class AbstractComponentPrivate
    {
    public:
        AbstractComponentPrivate(const QString & title)
            : title(title),
              isResizable(true)
            {}

        // Meta information
        QString title;
        // Component information
        AbstractComponent::DisplayFlags displayFlags;
        bool isResizable;
    };


    /**
     *  \brief Default component construction.
     */
    AbstractComponent::AbstractComponent(const QString & title)
        : QObject(), d(new AbstractComponentPrivate(title))
    {}

    /**
     *  \brief Destructor.
     */
    AbstractComponent::~AbstractComponent()
    {}

    /**
     *  \brief Get this component's parent view.
     */
    AlignmentView * AbstractComponent::alignmentView() const
    {
        return dynamic_cast< AlignmentView * >(QObject::parent());
    }

    AbstractComponent::DisplayFlags AbstractComponent::displayFlags() const
    {
        return d->displayFlags;
    }

    bool AbstractComponent::event(QEvent * event)
    {
        if (/* AlignmentView * av = */ alignmentView())
        {
            switch (event->type())
            {
            case QEvent::Leave:
                leaveEvent();
                break;

            case QEvent::MouseButtonDblClick:
                mouseDoubleClickEvent((QMouseEvent *) event);
                break;

            case QEvent::MouseButtonPress:
                mousePressEvent((QMouseEvent *) event);
                break;

            case QEvent::MouseButtonRelease:
                mouseReleaseEvent((QMouseEvent *) event);
                break;

            case QEvent::MouseMove:
                mouseMoveEvent((QMouseEvent *) event);
                break;

            case RenderEvent::Render:
                renderEvent((RenderEvent *) event);
                break;

            default:
                return QObject::event(event);
            }
        }

        return true;
    }

    /**
     *  \brief Query the geometry of this component within its parent view.
     */
    QRect AbstractComponent::geometry() const
    {
        return QRect(left(), top(), width(), height());
    }

    /**
     *  \brief Query if this component is resizable.
     */
    bool AbstractComponent::isResizable() const
    {
        return d->isResizable;
    }

    void AbstractComponent::leaveEvent() {}

    void AbstractComponent::mouseDoubleClickEvent(QMouseEvent * event)
    {
        event->ignore();
    }

    void AbstractComponent::mouseMoveEvent(QMouseEvent * event)
    {
        event->ignore();
    }

    void AbstractComponent::mousePressEvent(QMouseEvent * event)
    {
        event->ignore();
    }

    void AbstractComponent::mouseReleaseEvent(QMouseEvent * event)
    {
        event->ignore();
    }

    QPoint AbstractComponent::pos() const
    {
        return QPoint(left(), top());
    }

    QRect AbstractComponent::rect() const
    {
        return QRect(0, 0, width(), height());
    }

    void AbstractComponent::renderEvent(RenderEvent * event) {}

    void AbstractComponent::setDisplayFlags(DisplayFlags flags)
    {
        d->displayFlags = flags;
    }
    /**
     *  \brief Set if this component is resizable.
     */
    void AbstractComponent::setResizable(bool resizable)
    {
        d->isResizable = resizable;
    }

    /**
     *  \brief Set this component's title.
     *  \param title title to set.
     */
    void AbstractComponent::setTitle(const QString & title)
    {
        d->title = title;
        update();
    }

    QSize AbstractComponent::size() const
    {
        return QSize(width(), height());
    }

    void AbstractComponent::styleUpdate() {}

    /**
     *  \brief Query this component's title.
     *  \return this component's title.
     */
    QString AbstractComponent::title() const
    {
        return d->title;
    }

    void AbstractComponent::unitSizeUpdate(int) {}

    void AbstractComponent::zoomUpdate(double) {};

}
