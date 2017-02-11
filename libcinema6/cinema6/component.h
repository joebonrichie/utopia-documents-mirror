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

#ifndef COMPONENT_H
#define COMPONENT_H

#include <cinema6/config.h>
#include <cinema6/abstractcomponent.h>

#include <QPixmap>
#include <boost/scoped_ptr.hpp>

class QMouseEvent;
class QPaintDevice;

namespace CINEMA6
{

    class AlignmentView;

    class ComponentPrivate;
    class LIBCINEMA_API Component : public AbstractComponent
    {
        Q_OBJECT

    public:
        // Construction
        Component(const QString & title = QString());
        // Destruction
        virtual ~Component();

        // Properties
        int alignmentIndexAt(const QPoint & pos);
        virtual int height() const;
        double heightFactor() const;
        int left() const;
        QRectF rectAt(int index) const;
        void setHeight(int height);
        void setHeightFactor(double heightFactor);
        void setTop(int top);
        void setRelativeTop(int top);
        int top() const;
        void update();
        void updateGeometry();
        virtual int width() const;

    protected:
        // Rendering methods
        virtual void paint(QPainter * painter, const QRect & rect);
        virtual void render(QPaintDevice * target, const QPoint & targetOffset = QPoint(), const QRect & sourceRect = QRect());

        // Component events
        virtual void renderEvent(RenderEvent * event);

    protected slots:
        virtual void unitSizeUpdate(int size);
        virtual void zoomUpdate(double zoom);

    private:
        boost::scoped_ptr< ComponentPrivate > d;

    }; // class Component

    /**
     *  \brief Is this a component of the given type?
     */
    template< class _Component >
    bool isComponent(Component * component)
    {
        return (component != 0 && component == dynamic_cast< _Component* >(component));
    }

    // Convenience
    bool isComponent(const AbstractComponent * abstractComponent);

}

#endif // COMPONENT_H
