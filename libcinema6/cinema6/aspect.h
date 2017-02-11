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

#ifndef ASPECT_H
#define ASPECT_H

#include <cinema6/config.h>
#include <cinema6/abstractcomponent.h>

#include <boost/scoped_ptr.hpp>

class QMouseEvent;
class QPainter;

namespace CINEMA6
{

    class AlignmentView;
    class Component;

    class AspectPrivate;
    class LIBCINEMA_API Aspect : public AbstractComponent
    {
        Q_OBJECT

    public:
        // Construction
        Aspect(const QString & title = QString());
        // Destruction
        virtual ~Aspect();

        // Properties
        virtual int height() const;
        int left() const;
        void setWidth(int width);
        void setWidthFactor(double widthFactor);
        void setLeft(int left);
        int top() const;
        void update();
        void update(Component * component);
        virtual int width() const;
        double widthFactor() const;

    protected:
        // Rendering methods
        virtual void paint(QPainter * painter, const QRect & rect);
        virtual void paint(QPainter * painter, const QRect & rect, const QRect & clip, Component * component);

        // Aspect events
        bool event(QEvent * event);
        virtual void renderEvent(RenderEvent * event);

        // Component events
        using AbstractComponent::leaveEvent;
        virtual void leaveEvent(Component * component);
        virtual void mouseDoubleClickEvent(Component * component, QMouseEvent * event);
        virtual void mouseMoveEvent(Component * component, QMouseEvent * event);
        virtual void mousePressEvent(Component * component, QMouseEvent * event);
        virtual void mouseReleaseEvent(Component * component, QMouseEvent * event);

    private:

        using AbstractComponent::mouseDoubleClickEvent;
        using AbstractComponent::mouseMoveEvent;
        using AbstractComponent::mousePressEvent;
        using AbstractComponent::mouseReleaseEvent;

        boost::scoped_ptr< AspectPrivate > d;

    }; // class Aspect

    /**
     *  \brief Is this an aspect of the given type?
     */
    template< class _Aspect >
    bool isAspect(Aspect * aspect)
    {
        return (aspect != 0 && aspect == dynamic_cast< _Aspect* >(aspect));
    }

    // Convenience
    bool isAspect(const AbstractComponent * abstractComponent);

}

#endif // ASPECT_H
