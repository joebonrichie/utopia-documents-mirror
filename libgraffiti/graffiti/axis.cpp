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

#include <graffiti/axis.h>

namespace Graffiti
{

    Axis::Axis(QObject * parent)
        : QObject(parent)
    {
        this->initialise();
    }

    Axis::~Axis()
    {}

    QString Axis::legend() const
    {
        return this->_legend;
    }

    double Axis::max() const
    {
        return this->_max;
    }

    double Axis::min() const
    {
        return this->_min;
    }

    Axis::Orientation Axis::orientation() const
    {
        return this->_orientation;
    }

    QRectF Axis::rect() const
    {
        return this->_rect;
    }

    double Axis::resolve(double value)
    {
        double from;
        double to;
        switch (this->orientation())
        {
        case LeftToRight:
            from = this->rect().left();
            to = this->rect().right();
            break;
        case RightToLeft:
            from = this->rect().right();
            to = this->rect().left();
            break;
        case TopDown:
            from = this->rect().top();
            to = this->rect().bottom();
            break;
        case BottomUp:
            from = this->rect().bottom();
            to = this->rect().top();
            break;
        }

        if (from == to || this->min() == this->max())
        {
            return from;
        }
        else
        {
            return from + (value - this->min()) * (to - from) / (this->max() - this->min());
        }
    }

    void Axis::setLegend(QString legend)
    {
        if (this->_legend != legend)
        {
            this->_legend = legend;
            emit legendChanged(legend);
        }
    }

    void Axis::setMax(double max)
    {
        if (this->_max != max)
        {
            this->_max = max;
            emit maxChanged(max);
            emit rangeChanged(this->min(), max);
        }
    }

    void Axis::setMin(double min)
    {
        if (this->_min != min)
        {
            this->_min = min;
            emit minChanged(min);
            emit rangeChanged(min, this->max());
        }
    }

    void Axis::setOrientation(Orientation orientation)
    {
        if (this->_orientation != orientation)
        {
            this->_orientation = orientation;
            emit orientationChanged(orientation);
        }
    }

    void Axis::setRange(double min, double max)
    {
        bool minChanged = (this->_min != min);
        bool maxChanged = (this->_max != max);

        if (minChanged)
        {
            this->_min = min;
            emit this->minChanged(min);
        }
        if (maxChanged)
        {
            this->_max = max;
            emit this->maxChanged(max);
        }
        if (minChanged || maxChanged)
        {
            emit this->rangeChanged(min, max);
        }
    }

    void Axis::setRect(QRectF rect)
    {
        if (this->_rect != rect)
        {
            this->_rect = rect;
            emit rectChanged(rect);
        }
    }

    void Axis::setUnits(QString units)
    {
        if (this->_units != units)
        {
            this->_units = units;
            emit unitsChanged(units);
        }
    }

    QString Axis::units() const
    {
        return this->_units;
    }

    void Axis::initialise()
    {
        this->_min = 0;
        this->_max = 0;
    }

}
