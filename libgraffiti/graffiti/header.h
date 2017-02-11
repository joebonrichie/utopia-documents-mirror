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

#ifndef GRAFFITI_HEADER_H
#define GRAFFITI_HEADER_H

#include <QColor>
#include <QFrame>
#include <QVector>

namespace Graffiti
{

    class Sections;

    class HeaderPrivate;
    class Header : public QFrame
    {
        Q_OBJECT

        Q_PROPERTY(QColor cursorColor
                   READ cursorColor
                   WRITE setCursorColor)
        Q_PROPERTY(QColor gridColor
                   READ gridColor
                   WRITE setGridColor)
        Q_PROPERTY(QString orientation
                   READ orientation)

    public:
        Header(Qt::Orientation orientation, QWidget * parent = 0);
        Header(Sections * sections, Qt::Orientation orientation, QWidget * parent = 0);
        ~Header();

        Sections * sections();

        QColor cursorColor() const;
        double cursorValue() const;
        bool cursorVisible() const;
        QColor gridColor() const;
        QString orientation() const;
        bool reversed() const;
        void setCursorColor(const QColor & color);
        void setCursorValue(double value);
        void setCursorVisible(bool visible);
        void setGridColor(const QColor & color);
        void setOrientation(Qt::Orientation orientation);
        void setReversed(bool reversed);
        void setSections(Sections * sections);
        void setSnapToGuides(bool snap);
        void setSnapValues(const QVector< double > & values);
        void setVisibleRange(double from, double to);
        QVector< double > snapValues() const;
        int translateToWidget(double position) const;
        double translateFromWidget(int position) const;
        double visibleFrom() const;
        double visibleTo() const;

    protected:
        void contextMenuEvent(QContextMenuEvent * event);
        void enterEvent(QEvent * event);
        void leaveEvent(QEvent * event);
        void mouseMoveEvent(QMouseEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void mouseReleaseEvent(QMouseEvent * event);
        void paintEvent(QPaintEvent * event);

    signals:
        void cursorChanged(bool visible, double value);
        void dragCompleted();
        void dragInitiated(int boundary);
        void dragUpdated();
        void reversal();

    protected:
        HeaderPrivate * d;

    }; // class Header

} // namespace Graffiti

#endif // GRAFFITI_HEADER_H
