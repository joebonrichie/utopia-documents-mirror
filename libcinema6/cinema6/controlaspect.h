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

#ifndef CONTROLASPECT_H
#define CONTROLASPECT_H

#include <cinema6/config.h>
#include <cinema6/aspect.h>

namespace CINEMA6
{

    class LIBCINEMA_API ControlAspect : public Aspect
    {
        Q_OBJECT

    public:
        // Construction
        ControlAspect(const QString & title = QString());
        // Destruction
        virtual ~ControlAspect();

        // Properties
        virtual int width() const;

    protected:
        // Events
        virtual void leaveEvent(Component * component);
        virtual void mouseDoubleClickEvent(Component * component, QMouseEvent * event);
        virtual void mouseMoveEvent(Component * component, QMouseEvent * event);
        virtual void mousePressEvent(Component * component, QMouseEvent * event);
        virtual void mouseReleaseEvent(Component * component, QMouseEvent * event);
        virtual void paint(QPainter * painter, const QRect & rect, const QRect & clip, Component * component);

    private:
        // Hovering over?
        typedef enum
        {
            Nothing = 0,
            Close,
            Handle
        } HoverFocus;
        QMap< Component *, HoverFocus > _hoverFocus;

        // State of interaction
        typedef enum
        {
            Idle = 0,
            CloseButtonPressed
        } InteractionState;
        Component * _interactionComponent;

    }; // class ControlAspect

}

#endif // CONTROLASPECT_H
