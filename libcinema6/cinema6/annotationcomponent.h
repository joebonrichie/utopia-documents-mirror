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

#ifndef ANNOTATIONCOMPONENT_H
#define ANNOTATIONCOMPONENT_H

#include <cinema6/config.h>
#include <cinema6/datacomponent.h>
#include <cinema6/singleton.h>

LIBCINEMA_EXPORT inline bool operator < (const QSizeF & lhs, const QSizeF & rhs)
{
    if (lhs.width() != rhs.width())
        return lhs.width() < rhs.width();
    return lhs.height() < rhs.height();
}

namespace CINEMA6
{
    class SequenceComponent;

    class LIBCINEMA_API DoubleHelixPixmapFactory
    {
    public:
        // Get pixmap
        QPixmap foreground(QSizeF size);
        QPixmap background(QSizeF size);
        QPixmap base(QSizeF size, int index);

    private:
        // Cache
        QMap< QSizeF, QPixmap > _foregrounds;
        QMap< QSizeF, QPixmap > _backgrounds;
        QMap< QSizeF, QMap< int, QPixmap > > _bases;
    };

    class LIBCINEMA_API AnnotationComponent : public DataComponent
    {
        Q_OBJECT

        typedef Singleton< DoubleHelixPixmapFactory > PixmapFactory;

    public:
        // Construction
        AnnotationComponent(const QString & title = QString());
        // Destruction
        virtual ~AnnotationComponent();

        // Properties
        virtual int height() const;
        Sequence * sequence() const;
        SequenceComponent * sequenceComponent() const;
        void setSequenceComponent(SequenceComponent * sequenceComponent);

    protected:
        // Events
        virtual void paint(QPainter * painter, const QRect & rect);

    private:
        // Sequence for this annotation
        SequenceComponent * _sequenceComponent;

        PixmapFactory pixmapFactory;

    }; // class AnnotationComponent

}

#endif // ANNOTATIONCOMPONENT_H
