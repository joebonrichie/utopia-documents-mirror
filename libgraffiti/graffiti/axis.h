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

#ifndef GRAFFITI_AXIS_H
#define GRAFFITI_AXIS_H

#include <graffiti/config.h>
#include <QObject>
#include <QRectF>
#include <QString>

namespace Graffiti
{

    class LIBGRAFFITI_API Axis : public QObject
    {
        Q_OBJECT

    public:
        typedef enum
        {
            LeftToRight,
            RightToLeft,
            TopDown,
            BottomUp
        } Orientation;

        // Constructors
        Axis(QObject * parent = 0);
        // Destructor
        ~Axis();

        QString legend() const;
        double max() const;
        double min() const;
        Orientation orientation() const;
        inline double range() const { return this->max() - this->min(); }
        QRectF rect() const;
        double resolve(double value);
        void setLegend(QString legend);
        void setMax(double max);
        void setMin(double min);
        void setOrientation(Orientation orientation);
        void setRange(double min, double max);
        void setRect(QRectF rect);
        void setUnits(QString units);
        QString units() const;

    signals:
        void legendChanged(QString legend);
        void maxChanged(double max);
        void minChanged(double min);
        void orientationChanged(Graffiti::Axis::Orientation orientation);
        void rangeChanged(double min, double max);
        void rectChanged(QRectF rect);
        void unitsChanged(QString units);

    private:
        // members
        double _min;
        double _max;
        QString _legend;
        QString _units;

        Orientation _orientation;
        QRectF _rect;

        void initialise();

    }; // class GraphsWidget

}

#endif
