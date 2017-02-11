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

#ifndef SEQUENCECOMPONENT_H
#define SEQUENCECOMPONENT_H

#include <cinema6/config.h>
#include <cinema6/datacomponent.h>

#include <boost/scoped_ptr.hpp>

namespace CINEMA6
{
    class AnnotationComponent;
    class Sequence;

    class SequenceComponentPrivate;
    class LIBCINEMA_API SequenceComponent : public DataComponent
    {
        Q_OBJECT

    public:
        // Construction
        SequenceComponent(Sequence * sequence);
        // Destruction
        virtual ~SequenceComponent();

        // Properties
        Sequence * sequence() const;

        QString title() const;

    protected:
        // Events
        virtual void render(QPaintDevice * target, const QPoint & targetOffset = QPoint(), const QRect & sourceRect = QRect());

    protected slots:
        void dataChanged();

    private:
        boost::scoped_ptr< SequenceComponentPrivate > d;

        friend class AnnotationComponent;

    }; // class SequenceComponent

}

#endif // SEQUENCECOMPONENT_H
