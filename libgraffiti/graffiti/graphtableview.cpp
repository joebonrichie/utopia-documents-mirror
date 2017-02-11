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

#include <QDebug>
#include <QMouseEvent>

#include <graffiti/graphtableview.h>
#include <graffiti/tablewidget.h>

#include <QColorDialog>
#include <QHeaderView>
#include <QMenu>

#include <QDebug>

namespace Graffiti
{

    GraphTableView::GraphTableView(QWidget *parent, TableWidget *tableWidget) : QTableView(parent), _tableWidget(tableWidget)
    {
        this->verticalHeader()->hide();
        this->horizontalHeader()->hide();
        this->setMouseTracking(true);

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
                    emit colourChanged(modelIndex, QColorDialog::getColor(Qt::red));
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
                    QMenu menu;
                    QActionGroup * group = new QActionGroup(&menu);
                    group->setExclusive(true);
                    QAction * actionNotPlotted = group->addAction(menu.addAction("not plotted"));
                    actionNotPlotted->setCheckable(true);
                    QAction * actionXAxis = group->addAction(menu.addAction("X axis"));
                    actionXAxis->setCheckable(true);
                    QAction * actionYAxis = group->addAction(menu.addAction("Y axis"));
                    actionYAxis->setCheckable(true);
                    QAction * actionLabel = group->addAction(menu.addAction("Label"));
                    actionLabel->setCheckable(true);
                    menu.exec(mapToGlobal(visualRegion.bottomRight()));

                    QAction * checked = group->checkedAction();
                    if (checked == actionXAxis)
                    {
                        emit typeChanged(modelIndex, XAxis);
                    }
                    else if (checked == actionYAxis)
                    {
                        emit typeChanged(modelIndex, YAxis);
                    }
                    else if (checked == actionLabel)
                    {
                        emit typeChanged(modelIndex, Label);
                    }
                    else if (checked == actionNotPlotted)
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
