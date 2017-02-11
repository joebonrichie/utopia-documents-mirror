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

#include <graffiti/entrydelegate.h>
#include <QPainter>
#include <QColor>
#include <QFontMetrics>
#include <QLabel>
#include <QPalette>
#include <QCheckBox>

#include <QDebug>

#include <graffiti/tablewidget.h>

namespace Graffiti
{

    EntryDelegate::EntryDelegate(TableWidget *tableWidget) : _tableWidget(tableWidget)
    {}

    void EntryDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
    {
        QRect rect(option.rect);
        painter->fillRect(rect, QColor(220,220,220));

        if (this->_tableWidget->rowHeader() && index.row() < this->_tableWidget->headerRowCount() && this->_tableWidget->dataSeriesOrientation() == TableWidget::ColumnSeries)
        {
            // this is a Column Series table with a horizontal control header

            // fill the background of this header cell in black
            painter->fillRect(rect, Qt::black);

            // draw the label / title of this header at the top of this cell in white
            QFontMetrics metrics(painter->font());
            QString elidedLabel = metrics.elidedText(index.data().toString(), Qt::ElideRight, option.rect.width());
            QRect textRect(rect.left(), rect.top(), rect.width(), rect.height());
            painter->setPen(Qt::white);
            painter->drawText(textRect, elidedLabel, Qt::AlignTop | Qt::AlignHCenter);

            if (index.row() == this->_tableWidget->headerRowCount() -1)
            {
                //painter->fillRect(rect, Qt::blue);


                if (this->_tableWidget->graphType() == TableWidget::ScatterPlot)
                {

                    // for scatterplot graphs, a right hand icon allows selection between
                    // " " == don't draw this column
                    // "x" == draw this column on the graph's x axis
                    // "y" == draw this column on the graph's y axis
                    // "l" == use this column as the scatter points' labels
                    // the "l" option also allows a colour swatch to be selected for the points

                    QRect colourSquare(rect.bottomRight()+QPoint(-20,-16), QSize(12,12));
                    QColor colour= Qt::white;

                    painter->setBrush(colour);
                    painter->setPen(Qt::white);
                    painter->drawRect(colourSquare);
                    painter->setBrush(colour);
                    painter->setPen(Qt::black);

                    int type = index.data(Qt::UserRole).toInt();

                    if (type == GraphTableView::None)
                    {
                        painter->drawText(colourSquare, Qt::AlignCenter, " ");
                    }
                    else if (type == GraphTableView::XAxis)
                    {
                        painter->drawText(colourSquare, Qt::AlignCenter, "X");
                    }
                    else if (type == GraphTableView::YAxis)
                    {
                        painter->drawText(colourSquare, Qt::AlignCenter, "Y");
                    }
                    else if (type == GraphTableView::Label)
                    {
                        painter->drawText(colourSquare, Qt::AlignCenter, "L");

                        // if this is a label column, also draw the colour picker box
                        QRect colourSquare(rect.bottomLeft()+QPoint(5,-16), QSize(12,12));
                        QColor colour = index.data(Qt::UserRole+1).value<QColor>();
                        painter->setBrush(colour);
                        painter->setPen(Qt::white);
                        painter->drawRect(colourSquare);
                    }
                }
            }
            else // this is a linegraph
            {
//              qDebug() << "blah";
                /*
                // for line graphs, columns can either be selected or left unselected
                // so multiple lines can be plotted on the graph
                // if a column is selected, it also gets a colour swatch choice box
                // activated

                // draw an empty gray box bottom right
                QRect checkSquare(rect.bottomRight()+QPoint(-20,-16), QSize(12,12));
                painter->setBrush(Qt::gray);
                painter->setPen(Qt::white);
                painter->drawRect(checkSquare);

                bool checked =  index.data(Qt::UserRole + 2).toBool();

                // if this column is selected, draw a tiny check/tick in the box
                // and also draw the colour selection box
                if (checked)
                {
                painter->drawPixmap(checkSquare.topLeft()+QPoint(1,1), QPixmap(":/images/tinycheck.png"));


                QRect colourSquare(rect.bottomLeft()+QPoint(5,-16), QSize(12,12));
                QColor colour = index.data(Qt::UserRole+1).value<QColor>();
                painter->setBrush(colour);
                painter->setPen(Qt::white);
                painter->drawRect(colourSquare);
                }
                */
            }
        }
        /*
          else if (this->_tableWidget->columnHeader() && index.column() == 0 && this->_tableWidget->dataSeriesOrientation() == TableWidget::RowSeries)
          {
          // this is a Row Series table with a vertical control header
          QRect rect(option.rect);
          painter->fillRect(rect, Qt::black);


          QFontMetrics metrics(painter->font());
          QString elidedLabel = metrics.elidedText(index.data().toString(), Qt::ElideRight, option.rect.width() - 50);
          QRect textRect(rect.left()+25, rect.top(), rect.width()-50, rect.height());

          painter->setPen(Qt::white);
          painter->drawText(textRect, elidedLabel, Qt::AlignTop | Qt::AlignHCenter);


          if (this->_tableWidget->graphType() == TableWidget::ScatterPlot)
          {
          QRect colourSquare(rect.bottomRight()+QPoint(-20,-16), QSize(12,12));

          QColor colour = Qt::gray;

          painter->setBrush(colour);
          painter->setPen(Qt::white);
          painter->drawRect(colourSquare);

          painter->setBrush(colour);
          painter->setPen(Qt::black);

          int type = index.data(Qt::UserRole).toInt();

          if (type == GraphTableView::None)
          {
          painter->drawText(colourSquare, Qt::AlignCenter, " ");
          }
          else if (type == GraphTableView::XAxis)
          {
          painter->drawText(colourSquare, Qt::AlignCenter, "X");
          }
          else if (type == GraphTableView::YAxis)
          {
          painter->drawText(colourSquare, Qt::AlignCenter, "Y");
          }
          else if (type == GraphTableView::Label)
          {
          painter->drawText(colourSquare, Qt::AlignCenter, "L");

          // if this is a label column, also draw a colour picker box
          QRect colourSquare(rect.bottomLeft()+QPoint(5,-16), QSize(12,12));
          QColor colour = index.data(Qt::UserRole+1).value<QColor>();
          painter->setBrush(colour);
          painter->setPen(Qt::white);
          painter->drawRect(colourSquare);
          }


          }
          else
          {
          QRect checkSquare(rect.bottomRight()+QPoint(-20,-16), QSize(12,12));

          painter->setBrush(Qt::green);
          painter->setPen(Qt::white);
          painter->drawRect(checkSquare);

          bool checked =        index.data(Qt::UserRole + 2).toBool();

          if (checked)
          {
          painter->drawPixmap(checkSquare.topLeft()+QPoint(1,1), QPixmap(":/images/tinycheck.png"));

          QRect colourSquare(rect.bottomLeft()+QPoint(5,-16), QSize(12,12));
          QColor colour = index.data(Qt::UserRole+1).value<QColor>();
          painter->setBrush(colour);
          painter->setPen(Qt::white);
          painter->drawRect(colourSquare);
          }
          }

          }
        */
        else if ((this->_tableWidget->columnHeader() && index.column() == 0) || (this->_tableWidget->rowHeader() && index.row() == 0))
        {
            // this is a header cell, but not one that controls a data series
            painter->fillRect(rect, QColor(120,120,120));
            QStyledItemDelegate::paint(painter, option, index);
        }
        else
        {
            QStyledItemDelegate::paint(painter, option, index);
        }
    }

}
