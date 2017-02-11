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

#include <cinema6/alignmentview.h>
#include <cinema6/annotationcomponent.h>
#include <cinema6/component.h>
#include <cinema6/groupaspect.h>

#include <QMouseEvent>
#include <QPainter>

namespace CINEMA6
{

    /**
     *  \brief Default component construction.
     */
    GroupAspect::GroupAspect(const QString & title)
        : Aspect(title)
    {
        // GroupAspect objects are not resizable
        this->setResizable(false);
    }

    /**
     *  \brief Destructor.
     */
    GroupAspect::~GroupAspect()
    {}

    /**
     *  \brief Width of control aspect.
     */
    int GroupAspect::width() const
    {
        return 16;
    }

    /**
     *  \brief Paint control to screen.
     */
    void GroupAspect::paint(QPainter * painter, const QRect & rect, const QRect & /*clip*/, Component * component)
    {
        // Bail if not data
        if (!isComponent< DataComponent >(component)) { return; }

        int tabWidth = rect.width();
        int height = rect.height() - 1;
        bool isAnnotation = isComponent< AnnotationComponent >(component);
        int indent = isAnnotation ? 0 : 0;

        // Handle
        painter->setBrush(isAnnotation ? QColor(225, 125, 125) : QColor(165, 165, 255));
        painter->drawRect(indent, 0, tabWidth - indent, height);
    }

}
