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

#ifndef SELECTION_H
#define SELECTION_H

#include <cinema6/config.h>
#include <cinema6/selectionrange.h>

#include <QList>

namespace CINEMA6
{

    class LIBCINEMA_API Selection : public QList< SelectionRange >
    {
    public:
        enum SelectionFlag
        {
            Select,
            Deselect,
            Toggle
        };
        Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag);

        // Construction
        Selection();
        Selection(int from, int to);

        // properties
        bool contains(int index) const;
        QList< int > indices() const;
        void merge(const Selection & other, SelectionFlag command);
        void select(int from, int to);

        static void split(const SelectionRange & range, const SelectionRange & other, Selection * result);

    }; // class KeyComponent

    Q_DECLARE_OPERATORS_FOR_FLAGS(Selection::SelectionFlags);

}

Q_DECLARE_OPERATORS_FOR_FLAGS(CINEMA6::Selection::SelectionFlags);

#endif // SELECTION_H
