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

#ifndef GRAFFITI_GRID_H
#define GRAFFITI_GRID_H

#include <graffiti/cell.h>
#include <QObject>

namespace Graffiti
{

    class Sections;

    class GridPrivate;
    class Grid : public QObject
    {
        Q_OBJECT

    public:
        Grid(QObject * parent = 0);
        Grid(int rowCount, int columnCount, QObject * parent = 0);
        ~Grid();

        Cell at(int row, int column) const;
        int columnCount() const;
        void resize(int rowCount, int columnCount);
        int rowCount() const;

        void setHorizontalSections(Sections * sections);
        void setVerticalSections(Sections * sections);

    public slots:
        void insertColumn(int column);
        void insertColumns(int column, int count);
        void insertRow(int row);
        void insertRows(int row, int count);
        void removeColumn(int column);
        void removeColumns(int column, int count);
        void removeRow(int row);
        void removeRows(int row, int count);

    protected:
        GridPrivate * d;

    }; // class Grid

} // namespace Graffiti

#endif // GRAFFITI_GRID_H
