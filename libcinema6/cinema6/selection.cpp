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

#include <cinema6/selection.h>

namespace CINEMA6
{

    Selection::Selection()
        : QList< SelectionRange >()
    {}

    Selection::Selection(int from, int to)
        : QList< SelectionRange >()
    {
        this->select(from, to);
    }

    bool Selection::contains(int index) const
    {
        QListIterator< SelectionRange > iter(*this);
        while (iter.hasNext())
        {
            if (iter.next().contains(index))
            {
                return true;
            }
        }

        return false;
    }

    QList< int > Selection::indices() const
    {
        QList< int > indices;
        QListIterator< SelectionRange > iter(*this);
        while (iter.hasNext())
        {
            SelectionRange range = iter.next();
            for (int i = range.from(); i <= range.to(); ++i)
            {
                indices.append(i);
            }
        }

        return indices;
    }

    void Selection::merge(const Selection & other, SelectionFlag command)
    {
        Selection newSelection = other;
        Selection intersections;

        if (command != Select)
        {
            // Collect intersections or apply new selection
            Selection::iterator it = newSelection.begin();
            while (it != newSelection.end())
            {
                if (!(*it).isValid())
                {
                    it = newSelection.erase(it);
                    continue;
                }
                for (int t = 0; t < this->count(); ++t)
                {
                    if ((*it).intersects(this->at(t)))
                    {
                        intersections.append(this->at(t).intersected(*it));
                    }
                }
                ++it;
            }

            //  Split the old (and new) ranges using the intersections
            for (int i = 0; i < intersections.count(); ++i)
            { // for each intersection
                for (int t = 0; t < this->count();)
                { // splitt each old range
                    if (this->at(t).intersects(intersections.at(i)))
                    {
                        split(this->at(t), intersections.at(i), this);
                        this->removeAt(t);
                    }
                    else
                    {
                        ++t;
                    }
                }
                // only split newSelection if Toggle is specified
                for (int n = 0; (command == Toggle) && n < newSelection.count();)
                {
                    if (newSelection.at(n).intersects(intersections.at(i)))
                    {
                        split(newSelection.at(n), intersections.at(i), &newSelection);
                        newSelection.removeAt(n);
                    }
                    else
                    {
                        ++n;
                    }
                }
            }
        }

        if (command != Deselect)
        {
            Selection::iterator it = newSelection.begin();
            while (it != newSelection.end())
            {
                this->select((*it).from(), (*it).to());
                ++it;
            }
        }
    }

    void Selection::select(int from, int to)
    {
        SelectionRange selectRange(from, to);
        if (selectRange.isValid())
        {
            QMutableListIterator< SelectionRange > iter(*this);
            while (iter.hasNext())
            {
                SelectionRange range = iter.next();
                if (range.intersects(selectRange) || range.adjacent(selectRange))
                {
                    selectRange = range.united(selectRange);
                    iter.remove();
                }
            }

            this->append(selectRange);
        }
    }

    void Selection::split(const SelectionRange & range, const SelectionRange & other, Selection * result)
    {
        if (other.from() > range.from())
        {
            result->append(SelectionRange(range.from(), other.from() - 1));
        }
        if (other.to() < range.to())
        {
            result->append(SelectionRange(other.to() + 1, range.to()));
        }
    }

}
