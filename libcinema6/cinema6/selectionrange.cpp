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

#include <cinema6/selectionrange.h>

#include <QtGlobal>

namespace CINEMA6
{

    SelectionRange::SelectionRange()
    {
        this->setRange(-1, -1);
    }

    SelectionRange::SelectionRange(int from, int to)
    {
        this->setRange(from, to);
    }

    SelectionRange::SelectionRange(const SelectionRange & other)
    {
        this->setRange(other.from(), other.to());
    }

    bool SelectionRange::adjacent(const SelectionRange & other)
    {
        return this->isValid() && other.isValid() &&
            (this->from() == other.to() + 1 || other.from() == this->to() + 1);
    }

    bool SelectionRange::contains(int index) const
    {
        return this->isValid() && this->from() <= index && this->to() >= index;
    }

    int SelectionRange::from() const
    {
        return this->_from;
    }

    SelectionRange SelectionRange::intersected(const SelectionRange & other) const
    {
        int from = qMax(this->from(), other.from());
        int to = qMin(this->to(), other.to());
        return SelectionRange(from, to);
    }

    bool SelectionRange::intersects(const SelectionRange & other) const
    {
        return this->isValid() && other.isValid() &&
            this->from() <= other.to() && other.from() <= this->to();
    }

    bool SelectionRange::isValid() const
    {
        return this->from() <= this->to() && this->from() >= 0;
    }

    void SelectionRange::setFrom(int from)
    {
        this->setRange(from, this->to());
    }

    void SelectionRange::setRange(int from, int to)
    {
        // Ensure to > from
        if (from > to)
        {
            int tmp = from;
            from = to;
            to = tmp;
        }

        this->_from = from;
        this->_to = to;
    }

    void SelectionRange::setTo(int to)
    {
        this->setRange(this->from(), to);
    }

    int SelectionRange::size() const
    {
        return 1 + this->to() - this->from();
    }

    int SelectionRange::to() const
    {
        return this->_to;
    }

    SelectionRange SelectionRange::united(const SelectionRange & other) const
    {
        int from = qMin(this->from(), other.from());
        int to = qMax(this->to(), other.to());
        return SelectionRange(from, to);
    }

}

QDebug operator << (QDebug dbg, const CINEMA6::SelectionRange & range)
{
    dbg.nospace() << "(" << range.from() << "-" << range.to() << ")";
    return dbg.space();
}
