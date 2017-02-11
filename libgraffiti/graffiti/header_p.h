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

#ifndef GRAFFITI_HEADER_P_H
#define GRAFFITI_HEADER_P_H

#include <graffiti/sections.h>
#include <graffiti/header.h>

#include <QColor>
#include <QObject>
#include <QPen>

#include <set>

namespace Graffiti
{

    class HeaderPrivate : public QObject
    {
        Q_OBJECT

    public:
        typedef enum {
            Idle,
            Dragging
        } State;

        HeaderPrivate(Qt::Orientation orientation, Sections * sections, Header * header = 0);
        ~HeaderPrivate();

        Header * header;

        Sections * sections;
        void setSections(Sections * sections);
        Qt::Orientation orientation;

        void mousePositionChanged(const QPoint & point);
        void mouseMoved();

        struct {
            struct {
                QPoint pos;
                double value;
                int boundary;
            } press;
            struct {
                QPoint raw;
                QPoint pos;
                double value;
                int boundary;
            } move;
        } mouse;

        struct {
            bool enabled;
            bool possible;
            std::set< double > values;
            double nearest;
            bool available;
        } snap;

        struct {
            struct {
                double value;
            } press;
            struct {
                double value;
            } move;
        } snapped;

        struct {
        } guides;

        struct {
            bool isVisible;
            double value;
        } cursor;
        int draggingBoundary;
        double draggingOffset;
        bool context;
        State interactionState;

        struct {
            double from;
            double to;
        } visible;

        bool reversed;

        double visibleSize() const;

        int boundaryForValue(int mousePos, double value);

        QColor gridColor;
        QColor cursorColor;
        QPen guidePen;

        bool isSnappable() const;

        void setCursor(bool visible, double value);

        int breadth() const;
        int length() const;
        QPoint normalisedPos(const QPoint & point) const;

        int translateToWidget(double position) const;
        double translateFromWidget(int position) const;

    public slots:
        void onSplit();
        void onMerge();

        void onBoundariesChanged();
        void onBoundaryAdded(int i);
        void onBoundaryRemoved(int i);

    signals:
        void cursorChanged(bool visible, double value);

    }; // class HeaderPrivate

} // namespace Graffiti

#endif // GRAFFITI_HEADER_P_H
