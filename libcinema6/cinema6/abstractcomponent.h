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

#ifndef ABSTRACTCOMPONENT_H
#define ABSTRACTCOMPONENT_H

#include <cinema6/config.h>

#include <QObject>
#include <QPoint>
#include <QRect>
#include <boost/scoped_ptr.hpp>
#include <QSize>
#include <QString>

#include <QtDebug>

class QMouseEvent;

namespace CINEMA6
{

    class AlignmentView;
    class RenderEvent;

    class AbstractComponentPrivate;
    class LIBCINEMA_API AbstractComponent : public QObject
    {
        Q_OBJECT

    public:
        // Component state
        typedef enum
        {
            Display = 0,
            Minimised = 1
        } DisplayFlag;
        Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag);

        // Construction
        AbstractComponent(const QString & title = QString());
        // Destruction
        virtual ~AbstractComponent();

        // Properties
        AlignmentView * alignmentView() const;
        DisplayFlags displayFlags() const;
        QRect geometry() const;
        virtual int height() const = 0;
        bool isResizable() const;
        virtual int left() const = 0;
        QPoint pos() const;
        QRect rect() const;
        void setDisplayFlags(DisplayFlags flags);
        void setTitle(const QString & title);
        QSize size() const;
        QString title() const;
        virtual int top() const = 0;
        virtual void update() = 0;
        virtual int width() const = 0;

    protected:
        // Events
        bool event(QEvent * event);
        virtual void leaveEvent();
        virtual void mouseDoubleClickEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent * event);
        virtual void mousePressEvent(QMouseEvent * event);
        virtual void mouseReleaseEvent(QMouseEvent * event);
        virtual void renderEvent(RenderEvent * event);

        // Properties
        void setResizable(bool resizable);

    public slots:
        // Updates of view
        virtual void styleUpdate();
        virtual void unitSizeUpdate(int size);
        virtual void zoomUpdate(double zoom);

    private:
        boost::scoped_ptr< AbstractComponentPrivate > d;

    }; // class AbstractComponent

}

Q_DECLARE_OPERATORS_FOR_FLAGS(CINEMA6::AbstractComponent::DisplayFlags);

#endif // ABSTRACTCOMPONENT_H
