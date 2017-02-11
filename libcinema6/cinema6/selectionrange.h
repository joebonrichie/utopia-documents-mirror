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

#ifndef SELECTIONRANGE_H
#define SELECTIONRANGE_H

#include <cinema6/config.h>

#include <QDebug>

namespace CINEMA6
{

    class LIBCINEMA_API SelectionRange
    {
    public:
        // Construction
        SelectionRange();
        SelectionRange(int from, int to);
        SelectionRange(const SelectionRange & other);

        // properties
        bool adjacent(const SelectionRange & other);
        bool contains(int index) const;
        int from() const;
        SelectionRange intersected(const SelectionRange & other) const;
        bool intersects(const SelectionRange & other) const;
        bool isValid() const;
        void setFrom(int from);
        void setRange(int from, int to);
        void setTo(int to);
        int size() const;
        int to() const;
        SelectionRange united(const SelectionRange & other) const;

    private:
        // Index range
        int _from;
        int _to;

    }; // class SelectionRange

}

QDebug operator << (QDebug dbg, const CINEMA6::SelectionRange & range);

#endif // SELECTIONRANGE_H
