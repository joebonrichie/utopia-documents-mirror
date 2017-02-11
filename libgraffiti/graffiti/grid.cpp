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

#include <graffiti/grid.h>
#include <graffiti/grid_p.h>
#include <graffiti/sections.h>

namespace Graffiti
{

    GridPrivate::GridPrivate(Grid * grid)
        : QObject(grid),
          grid(grid),
          horizontalSections(0),
          verticalSections(0),
          rowCount(0),
          columnCount(0)
    {}

    GridPrivate::~GridPrivate()
    {}

    void GridPrivate::onHorizontalSectionsBoundaryAdded(int i)
    {
    }

    void GridPrivate::onHorizontalSectionsBoundaryRemoved(int i)
    {
    }

    void GridPrivate::onHorizontalSectionsDestroyed(QObject * obj)
    {
        grid->setHorizontalSections(0);
    }

    void GridPrivate::onVerticalSectionsBoundaryAdded(int i)
    {
    }

    void GridPrivate::onVerticalSectionsBoundaryRemoved(int i)
    {
    }

    void GridPrivate::onVerticalSectionsDestroyed(QObject * obj)
    {
        grid->setVerticalSections(0);
    }




    Grid::Grid(QObject * parent)
        : QObject(parent), d(new GridPrivate(this))
    {}

    Grid::Grid(int rowCount, int columnCount, QObject * parent)
        : QObject(parent), d(new GridPrivate(this))
    {
        resize(rowCount, columnCount);
    }

    Grid::~Grid()
    {}

    Cell Grid::at(int row, int column) const
    {
        if (row >= 0 && row < d->rowCount) {
            if (column >= 0 && column < d->columnCount) {
                return d->cells[row][column];
            }
        }

        return NullCell();
    }

    int Grid::columnCount() const
    {
        return d->columnCount;
    }

    void Grid::insertColumn(int column)
    {
        QMutableListIterator< QList< Cell > > iter(d->cells);
        while (iter.hasNext()) {
            QList< Cell > & row(iter.next());
            row.insert(qBound(0, column, d->columnCount), Cell());
        }
        ++d->columnCount;
    }

    void Grid::insertColumns(int column, int count)
    {
        while (count > 0) {
            insertColumn(column);
            --count;
        }
    }

    void Grid::insertRow(int row)
    {
        QList< Cell > newRow;
        // Extend row if necessary
        while (newRow.count() < d->columnCount) {
            newRow.push_back(Cell());
        }
        d->cells.insert(qBound(0, row, d->rowCount), newRow);
        ++d->rowCount;
    }

    void Grid::insertRows(int row, int count)
    {
        while (count > 0) {
            insertRow(row);
            --count;
        }
    }

    void Grid::removeColumn(int column)
    {
        if (column >= 0 && column < d->columnCount) {
            QMutableListIterator< QList< Cell > > iter(d->cells);
            while (iter.hasNext()) {
                QList< Cell > & row(iter.next());
                row.removeAt(column);
            }
            --d->columnCount;
        }
    }

    void Grid::removeColumns(int column, int count)
    {
        while (count > 0) {
            removeColumn(column);
            --count;
        }
    }

    void Grid::removeRow(int row)
    {
        if (row >= 0 && row < d->rowCount) {
            d->cells.removeAt(row);
            --d->rowCount;
        }
    }

    void Grid::removeRows(int row, int count)
    {
        while (count > 0) {
            removeRow(row);
            --count;
        }
    }

    void Grid::resize(int rowCount, int columnCount)
    {
        d->rowCount = qMax(rowCount, 0);
        d->columnCount = qMax(columnCount, 0);

        // Remove excess rows
        while (d->cells.count() > d->rowCount) {
            d->cells.pop_back();
        }

        // Add extra rows
        while (d->cells.count() < d->rowCount) {
            d->cells.push_back(QList< Cell >());
        }

        QMutableListIterator< QList< Cell > > iter(d->cells);
        while (iter.hasNext()) {
            QList< Cell > & row(iter.next());

            // Remove excess columns
            while (row.count() > d->columnCount) {
                row.pop_back();
            }

            // Extend rows if necessary
            while (row.count() < d->columnCount) {
                row.push_back(Cell());
            }
        }
    }

    int Grid::rowCount() const
    {
        return d->rowCount;
    }

    void Grid::setHorizontalSections(Sections * sections)
    {
        if (d->horizontalSections) {
            d->horizontalSections->disconnect(this);
        }
        d->horizontalSections = sections;
        if (d->horizontalSections) {
            connect(d->horizontalSections, SIGNAL(boundaryAdded(int)),
                    d, SLOT(onHorizontalSectionsBoundaryAdded(int)));
            connect(d->horizontalSections, SIGNAL(boundaryRemoved(int)),
                    d, SLOT(onHorizontalSectionsBoundaryRemoved(int)));
            connect(d->horizontalSections, SIGNAL(destroyed(QObject*)),
                    d, SLOT(onHorizontalSectionsDestroyed(QObject*)));
        }
    }

    void Grid::setVerticalSections(Sections * sections)
    {
        if (d->verticalSections) {
            d->verticalSections->disconnect(this);
        }
        d->verticalSections = sections;
        if (d->verticalSections) {
            connect(d->verticalSections, SIGNAL(boundaryAdded(int)),
                    d, SLOT(onVerticalSectionsBoundaryAdded(int)));
            connect(d->verticalSections, SIGNAL(boundaryRemoved(int)),
                    d, SLOT(onVerticalSectionsBoundaryRemoved(int)));
            connect(d->verticalSections, SIGNAL(destroyed(QObject*)),
                    d, SLOT(onVerticalSectionsDestroyed(QObject*)));
        }
    }

} // namespace Graffiti
