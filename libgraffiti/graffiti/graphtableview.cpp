/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <QDebug>
#include <QMouseEvent>
#include "qtcolorpicker.h"

#include <graffiti/graphtableview.h>
#include <graffiti/tablewidget.h>
#include <QHeaderView>
#include <QDebug>

namespace Graffiti
{

    GraphTableView::GraphTableView(QWidget *parent, TableWidget *tableWidget) : QTableView(parent), _tableWidget(tableWidget)
    {
        this->verticalHeader()->hide();
        this->horizontalHeader()->hide();
        this->setMouseTracking(true);
        this->_colourPicker = 0;
        this->_typePicker = 0;

        connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(cellClicked(const QModelIndex &)));
    }

    void GraphTableView::mousePressEvent(QMouseEvent *event)
    {
        QModelIndex modelIndex = this->indexAt(event->pos());
        QRect visualRegion = this->visualRect(modelIndex);
        this->QTableView::mousePressEvent(event);

        qDebug() << "COLS : " << this->_tableWidget->headerRowCount();

        if ((this->_tableWidget->dataSeriesOrientation() == TableWidget::ColumnSeries && modelIndex.row() == this->_tableWidget->headerRowCount() -1) ||
            (this->_tableWidget->dataSeriesOrientation() == TableWidget::RowSeries && modelIndex.column() == 0))
        {

            if ((event->pos().x() < (visualRegion.left() + 20)) && (event->pos().y() > (visualRegion.bottom() - 20)))
            {
                if ((this->model()->data(modelIndex, Qt::UserRole).toInt() == GraphTableView::Label)
                    ||
                    (this->model()->data(modelIndex, Qt::UserRole + 2).toBool() == true))
                {
                    if (this->_colourPicker == 0)
                    {
                        this->_colourPicker = new QtColorPicker(this);
                        this->_colourPicker->setStandardColors();
                        this->_colourPicker->setColorDialogEnabled(true);
                        this->_colourPicker->hide();
                    }

                    QColor newColour = this->_colourPicker->getColorFromPopup(this->mapToGlobal(visualRegion.bottomLeft()));

                    this->_colourPicker->setCurrentColor(newColour);
                    emit colourChanged(modelIndex, newColour);
                }
            }
            else if ((event->pos().x() > (visualRegion.right() - 20)) && (event->pos().y() > (visualRegion.bottom() - 20)))
            {
                if (this->_tableWidget->graphType() == TableWidget::LineGraph)
                {
                    emit checkChanged(modelIndex);
                }
                else
                {
                    // horrible horrible hack, which uses the first char of these tooltips as the character
                    // to plot in the selection box, i.e. ' ', 'X','Y' or 'L'
                    if (this->_typePicker == 0)
                    {
                        this->_typePicker= new QtColorPicker(this, -1, false);
                        this->_typePicker->setLabels(true);
                        this->_typePicker->insertColor(Qt::cyan, " not plotted");
                        this->_typePicker->insertColor(Qt::red, "X axis");
                        this->_typePicker->insertColor(Qt::green, "Y axis");
                        this->_typePicker->insertColor(Qt::blue, "Label");
                        this->_typePicker->hide();
                    }


                    QColor newType = this->_typePicker->getColorFromPopup(this->mapToGlobal(visualRegion.bottomRight()));
                    if (newType == Qt::red)
                    {
                        emit typeChanged(modelIndex, XAxis);
                    }
                    else if (newType == Qt::green)
                    {
                        emit typeChanged(modelIndex, YAxis);
                    }
                    else if (newType == Qt::blue)
                    {
                        emit typeChanged(modelIndex, Label);
                    }
                    else if (newType == Qt::cyan)
                    {

                        emit typeChanged(modelIndex, None);
                    }
                }
            }
        }

    }

    void GraphTableView::setCurrentColor(const QColor &)
    {
    }

    void GraphTableView::cellClicked(const QModelIndex &)
    {
    }

    void GraphTableView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
    {
        emit dataChanged();
    }

}
