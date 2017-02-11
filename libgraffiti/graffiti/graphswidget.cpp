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
#include <graffiti/graphswidget.h>

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QUrl>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QDebug>


#include <math.h>
#include <string.h>


#include <graffiti/tablewidget.h>
#include <graffiti/graphtableview.h>

namespace Graffiti
{

    GraphsWidget::GraphsWidget(QWidget * parent, Qt::WindowFlags f, QStandardItemModel *model)
        : QWidget(parent, f), _bubblePixmap(NULL), _bubbleTitle(NULL), _model(model)
    {
        this->initialise();

        this->_bubbleIndex = -1;
    }

    GraphsWidget::~GraphsWidget()
    {}

    void GraphsWidget::initialise()
    {
        // Layout graphics view
        QHBoxLayout * layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        this->_view = new QGraphicsView;
        layout->addWidget(this->_view);
        this->_scene = new QGraphicsScene;
        //this->_scene->installEventFilter(this);
        this->_view->viewport()->installEventFilter(this);
        this->_view->setScene(this->_scene);
        this->_view->setRenderHint(QPainter::Antialiasing);
        this->_view->setFrameStyle(QFrame::NoFrame);
        //this->_view->setDragMode(QGraphicsView::ScrollHandDrag);
        //this->_view->setDragMode(QGraphicsView::RubberBandDrag);


        this->setMouseTracking(true);

        this->_xAxis = new Axis(this);
        this->_xAxis->setOrientation(Axis::LeftToRight);
        this->_xAxis->setRange(0, 5);

        this->_yAxis = new Axis(this);
        this->_yAxis->setOrientation(Axis::BottomUp);
        this->_yAxis->setRange(0, 120);

        QObject::connect(this->_xAxis, SIGNAL(rectChanged(QRectF)), this, SLOT(xAxisRectChanged(QRectF)));
        QObject::connect(this->_yAxis, SIGNAL(rectChanged(QRectF)), this, SLOT(yAxisRectChanged(QRectF)));

        // Hard wired - REMOVE FIXME
        double x[21] = {
            0.86,
            0.91,
            1.37,
            0.32,
            2.09,
            2.15,
            2.27,
            2.75,
            3.1,
            3.97,
            4.28,
            3.74,
            3.97,
            4.81,
            4.05,
            4.55,
            4.88,
            4.16,
            4.91,
            4.27,
            4.11
        };
        memcpy(this->_x, x, sizeof(double) * 21);

        double y[21] = {
            13,
            21,
            32,
            41,
            52,
            55,
            55,
            60,
            70,
            90,
            91,
            92,
            93,
            95,
            97,
            97,
            98,
            98,
            100,
            100,
            100
        };
        memcpy(this->_y, y, sizeof(double) * 21);

        for (int i = 0;i<21;++i)
        {
            this->_yData.append(y[i]);
            this->_xData.append(x[i]);
        }

        this->setZoomMode(FitToWindow);

        this->_oldDataSeries = TableWidget::ColumnSeries;
        this->_oldType = TableWidget::ScatterPlot;
    }

    void GraphsWidget::activate(QString name)
    {
        if (!this->_linkTemplate.isEmpty())
        {
            QDesktopServices::openUrl(QUrl(QString(this->_linkTemplate).arg(name)));
        }
    }


    bool GraphsWidget::eventFilter(QObject * obj, QEvent * event)
    {
        if (event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            QGraphicsSceneMouseEvent * sceneEvent = dynamic_cast< QGraphicsSceneMouseEvent * >(event);
            if (sceneEvent)
            {
                QGraphicsItem * item = this->_view->itemAt(this->_view->mapFromScene(sceneEvent->scenePos()));
                if (item)
                {
                    this->activate(item->toolTip());
                }
            }
        }
        else if (event->type() == QEvent::Resize && obj == this->_view->viewport())
        {
            QSize size = this->_view->viewport()->size();
            this->_xAxis->setRect(QRectF(50, size.height() - 50, size.width() - 100, 0));
            this->_yAxis->setRect(QRectF(50, 50, 0, size.height() - 100));
            this->_view->setSceneRect(this->_view->viewport()->rect());
        }

        return this->QWidget::eventFilter(obj, event);
    }


    void GraphsWidget::resizeScene()
    {
        switch (this->zoomMode())
        {
        case FitToWindow:
        {
            this->_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            this->_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            QSize size = this->size();
            this->_xAxis->setRect(QRectF(50, size.height() - 50, size.width() - 100, 0));
            this->_yAxis->setRect(QRectF(50, 50, 0, size.height() - 100));
            this->_view->setSceneRect(this->rect());
            this->dataChanged(this->_oldDataSeries, this->_oldType, this->_headerRowCount);
            this->update();
        }
        break;
        case CustomZoom:
            this->_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            this->_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            this->_view->setMatrix(QMatrix());
            this->_view->scale(this->zoom(), this->zoom());
            break;
        default:
            break;
        }
    }

    void GraphsWidget::dataChanged(TableWidget::DataSeriesOrientation dataSeries, TableWidget::GraphType type, int headerRowCount)
    {
        _oldDataSeries = dataSeries;
        _oldType = type;
        this->_headerRowCount = headerRowCount;
        //qDebug() << " ************** DATA CHANGED IN GRAPH headerrowcount = " << this->_headerRowCount;
        //qDebug() << " dataSeries: " << dataSeries << " type: " << type;


        if (type == TableWidget::ScatterPlot && dataSeries == TableWidget::ColumnSeries)
        {
            // FIXME
            // if we can find exactly one each of X axis, Y axis and Label roles in the 0th row, then reset the data
            //qDebug() << "Here!" << this->_model->columnCount();
            int xAxisColumn = -1;
            int yAxisColumn = -1;
            int labelColumn = -1;
            for (int column = 0; column < this->_model->columnCount() ; ++column)
            {
                QModelIndex index = this->_model->index(this->_headerRowCount - 1, column);
                int type = index.data(Qt::UserRole).toInt();
                if (type == GraphTableView::XAxis)
                {
                    //qDebug() << "Found x axis column";
                    xAxisColumn = column;
                }
                else if (type == GraphTableView::YAxis)
                {
                    //qDebug() << "Found y axis column";
                    yAxisColumn = column;
                }
                else if (type == GraphTableView::Label)
                {
                    //qDebug() << "Found label column";
                    labelColumn = column;
                }
            }

            if (xAxisColumn != -1 && yAxisColumn != -1 && labelColumn != -1)
            {

                this->_view->scene()->clear();
                this->_bubbleTitle = NULL;
                this->_bubblePixmap = NULL;
                /*
                // remove current scene contents
                for (int i = 0 ; i < this->_view->scene()->count(); ++i)
                {
                QGraphicsViewItem *item = this->_view->scene().items(a
                this->_view->scene()->removeItem(items);
                delete *items;
                items.next();
                }
                */
                this->_xData.clear();
                this->_yData.clear();
                this->_labelData.clear();

                QColor nodeColour = this->_model->data(this->_model->index(this->_headerRowCount - 1,labelColumn), Qt::UserRole+1).value<QColor>();
                QString xAxisLabel = this->_model->data(this->_model->index(this->_headerRowCount - 1,xAxisColumn)).toString();
                QString yAxisLabel = this->_model->data(this->_model->index(this->_headerRowCount - 1,yAxisColumn)).toString();

                double xMax, yMax, xMin, yMin;
                for (int row = this->_headerRowCount; row < this->_model->rowCount(); ++row)
                {
                    QModelIndex xindex= this->_model->index(row, xAxisColumn);
                    QString data = xindex.data().toString();
                    if (data.contains(QChar(0x00B1)))
                    {
                        //qDebug() << "Got a plusminus";
                        int index = data.indexOf(QChar(0x00B1));
                        data = data.left(index);
                        //qDebug() << "Stripped on create to " << data;
                    }

                    double xValue = data.toDouble();

                    QModelIndex yindex= this->_model->index(row, yAxisColumn);
                    data = yindex.data().toString();
                    if (data.contains(QChar(0x00B1)))
                    {
                        //qDebug() << "Got a plusminus";
                        int index = data.indexOf(QChar(0x00B1));
                        data = data.left(index);
                        //qDebug() << "Stripped on create to " << data;
                    }

                    double yValue = data.toDouble();

                    QModelIndex labelindex= this->_model->index(row, labelColumn);
                    QString label = labelindex.data().toString();


                    if (row == this->_headerRowCount )
                    {
                        xMax = xValue;
                        yMax = yValue;
                        xMin = xValue;
                        yMin = yValue;
                    }
                    else
                    {
                        if (xValue > xMax) xMax = xValue;
                        if (yValue > yMax) yMax = yValue;
                        if (xValue < xMin) xMin = xValue;
                        if (yValue < yMin) yMin = yValue;
                    }

                    this->_xData.append(xValue);
                    this->_yData.append(yValue);
                    this->_labelData.append(label);
                }

                if (xMin != 0)
                {
                    if (xMin < 0)
                    {
                        xMin = -xMin;
                        double newxMin = pow(10, floor(log10(xMin)));
                        xMin = ceil(xMin / newxMin) * newxMin;
                        xMin = -xMin;
                    }
                    else
                    {
                        double newxMin = pow(10, floor(log10(xMin)));
                        xMin = floor(xMin / newxMin) * newxMin;
                    }
                }

                if (xMax != 0)
                {
                    if (xMax < 0)
                    {
                        double newxMax = pow(10, floor(log10(-xMax)));
                        xMax = - ceil(xMax / newxMax) * newxMax;
                    }
                    else
                    {
                        double newxMax = pow(10, floor(log10(xMax)));
                        xMax = ceil(xMax / newxMax) * newxMax;
                    }
                }



                if (yMin != 0)
                {
                    if (yMin < 0)
                    {
                        yMin = -yMin;
                        double newyMin =  pow(10, floor(log10(yMin)));
                        yMin = ceil(yMin / newyMin) * newyMin;
                        yMin = -yMin;
                    }
                    else
                    {
                        double newyMin = pow(10, floor(log10(yMin)));
                        yMin = floor(yMin / newyMin) * newyMin;
                    }
                }

                if (yMax != 0)
                {
                    if (yMax < 0)
                    {
                        double newyMax = pow(10, floor(log10(-yMax)));
                        yMax = -ceil(yMax / newyMax) * newyMax;
                    }
                    else
                    {
                        double newyMax = pow(10, floor(log10(yMax)));
                        yMax = ceil(yMax / newyMax) * newyMax;
                    }
                }



                this->_xAxis->setRange(xMin, xMax);



                this->_yAxis->setRange(yMin, yMax);


                double labelWidth = 50;
                double labelHeight = 20;

                double xlength = this->_xAxis->rect().width();
                double maxXLabels = xlength / labelWidth;
                //                      double rawXInterval = this->_xAxis->range() / maxXLabels;
                double xlog = log10(this->_xAxis->range() / maxXLabels);
                double xinterval = pow(10, ceil(xlog));


                double ylength = this->_yAxis->rect().height();
                double maxYLabels = ylength / labelHeight;
                //                      double rawYInterval = this->_yAxis->range() / maxYLabels;
                double ylog = log10(this->_yAxis->range() / maxYLabels);
                double yinterval = pow(10, ceil(ylog));





                // Add points to scene



                double horizontalAxis = 0;
                if (xMin > 0)
                {
                    horizontalAxis = xMin;
                }

                double verticalAxis = 0;
                if (yMin > 0)
                {
                    verticalAxis = yMin;
                }


                if (xinterval > 0) {
                    for (double xLabelIndex = xMin; xLabelIndex <= xMax; xLabelIndex += xinterval)
                    {
                        QRect tick(this->_xAxis->resolve(xLabelIndex), this->_yAxis->resolve(verticalAxis),0,10);
                        this->_view->scene()->addRect(tick);

                        QGraphicsSimpleTextItem *label= this->_view->scene()->addSimpleText(QString::number(xLabelIndex));
                        QRectF textRect = label->boundingRect();


                        label->setPos(this->_xAxis->resolve(xLabelIndex) - (textRect.width() / 2.0), this->_yAxis->resolve(verticalAxis)+10);
                    }
                }

                float yLabelMinDist;

                if (yinterval > 0) {
                    for (double yLabelIndex = yMin; yLabelIndex <= yMax; yLabelIndex += yinterval)
                    {
                        QRect tick(this->_xAxis->resolve(horizontalAxis)-10, this->_yAxis->resolve(yLabelIndex), 10, 0);
                        this->_view->scene()->addRect(tick);

                        QGraphicsSimpleTextItem *label= this->_view->scene()->addSimpleText(QString::number(yLabelIndex));
                        QRectF textRect = label->boundingRect();


                        double xdist = this->_xAxis->resolve(horizontalAxis) - textRect.width() - 15;

                        label->setPos(xdist, this->_yAxis->resolve(yLabelIndex) - (textRect.height() / 2.0));

                        if (yLabelIndex == yMin)
                        {
                            yLabelMinDist = xdist;
                        }
                        else if (xdist < yLabelMinDist)
                        {
                            yLabelMinDist = xdist;
                        }
                    }
                }

                //this->_view->scene()->addRect(this->_xAxis->rect().left(), this->_yAxis->resolve(verticalAxis), this->_xAxis->rect().width(), 0);
                this->_view->scene()->addLine(this->_xAxis->rect().left(),
                                              this->_yAxis->resolve(verticalAxis),
                                              this->_xAxis->rect().left()+this->_xAxis->rect().width(),
                                              this->_yAxis->resolve(verticalAxis), QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap));

                this->_view->scene()->addLine(this->_xAxis->resolve(horizontalAxis),
                                              this->_yAxis->rect().top(),
                                              this->_xAxis->resolve(horizontalAxis),
                                              this->_yAxis->rect().top() + this->_yAxis->rect().height(), QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap));


                for (int i = 0; i < this->_xData.size(); ++i)
                {
                    QGraphicsItem * item = this->_view->scene()->addEllipse(this->_xAxis->resolve(this->_xData[i]) - 3, this->_yAxis->resolve(this->_yData[i]) - 3, 6, 6, QPen(Qt::black), nodeColour);
                    if (this->_labelData.size() == this->_xData.size())
                    {
                        item->setToolTip(this->_labelData[i]);
                    }
                }

                QGraphicsSimpleTextItem *xaxislabel = this->_view->scene()->addSimpleText(xAxisLabel);
                QRectF xtextRect = xaxislabel->boundingRect();
                xaxislabel->setPos(((this->_xAxis->rect().bottomLeft().x() + this->_xAxis->rect().bottomRight().x()) / 2.0) - xtextRect.width() / 2.0, this->_xAxis->rect().bottom() + 20);

                QGraphicsSimpleTextItem *yaxislabel = this->_view->scene()->addSimpleText(yAxisLabel);
                yaxislabel->setRotation(-90);
                QRectF ytextRect = yaxislabel->boundingRect();
                //              yaxislabel->setPos(this->_xAxis->rect().left() - 50, ((this->_yAxis->rect().topLeft().y() + this->_xAxis->rect().bottomLeft().y()) / 2.0) + ytextRect.width() / 2.0);
                yaxislabel->setPos(yLabelMinDist - 20, ((this->_yAxis->rect().topLeft().y() + this->_xAxis->rect().bottomLeft().y()) / 2.0) + ytextRect.width() / 2.0);




            }
            else
            {
            }
        }
        else
        {
        }

        update();
    }

    void GraphsWidget::resizeEvent(QResizeEvent * e)
    {
        if (this->zoomMode() == FitToWindow)
        {
            this->resizeScene();
        }
        emit resized();
    }

    void GraphsWidget::setLinkTemplate(const QString & linkTemplate)
    {
        this->_linkTemplate = linkTemplate;
    }

    void GraphsWidget::setZoom(double zoom)
    {
        this->_zoom = zoom;
        this->setZoomMode(CustomZoom);
        this->resizeScene();
    }

    void GraphsWidget::setZoomMode(ZoomMode mode)
    {
        if (this->_zoomMode != mode)
        {
            this->_zoomMode = mode;
            if (mode == FitToWindow)
            {
                this->_zoom = 1;
            }
            this->resizeScene();
        }
    }

    void GraphsWidget::wheelEvent(QWheelEvent * event)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            double newZoom = this->zoom() + event->delta() / 12000.0;
            if (newZoom < 0.1) { newZoom = 0.1; }
            if (newZoom > 50.0) { newZoom = 50.0; }
            this->setZoom(newZoom);
        }
        else
        {
            this->QWidget::wheelEvent(event);
        }
    }

    void GraphsWidget::mousePressEvent(QMouseEvent * event)
    {
        QPoint bubbleAdjust(100,120);

        int oldBubbleIndex = this->_bubbleIndex;

        if (this->_bubblePixmap != NULL)
        {
            this->_view->scene()->removeItem(this->_bubblePixmap);
            this->_view->scene()->removeItem(this->_bubbleTitle);
            delete this->_bubblePixmap;
            delete this->_bubbleTitle;
            this->_bubblePixmap = NULL;
            this->_bubbleTitle = NULL;
            this->_bubbleIndex = -1;
        }

        bool found = false;
        for (int i = 0; i < this->_xData.size() && !found; ++i)
        {
            QRect nodeRect(this->_xAxis->resolve(this->_xData[i]) - 6, this->_yAxis->resolve(this->_yData[i]) - 6, 12, 12);
            //this->_view->scene()->addRect(nodeRect);
            if (nodeRect.contains(event->pos()))
            {
                this->_bubblePixmap= this->_view->scene()->addPixmap(QPixmap(":/images/graphbubble.png"));
                this->_bubblePixmap->setZValue(1);
                this->_bubblePixmap->setPos(QPoint(this->_xAxis->resolve(this->_xData[i]),this->_yAxis->resolve(this->_yData[i]))-bubbleAdjust);
                this->_bubblePixmap->setOpacity(0.9);

                this->_bubbleTitle= this->_view->scene()->addSimpleText(this->_labelData[i]);
                double textWidth= this->_bubbleTitle->boundingRect().width();
                this->_bubbleTitle->setZValue(2);
                this->_bubbleTitle->setPos(QPoint(this->_xAxis->resolve(this->_xData[i]),this->_yAxis->resolve(this->_yData[i]))-bubbleAdjust+QPoint(80-textWidth/2.0,45));

                this->_bubbleIndex = i;

                found = true;
            }
        }

        // HACK .. can't get doubleClickEvents through this far for some reason
        // so for now, remember which bubble was activated before, and if it's activateed
        // a second time, then open up the CHEBI link
        if (this->_bubbleIndex == oldBubbleIndex && this->_bubbleIndex != -1)
        {
            this->activate(this->_labelData[this->_bubbleIndex]);
        }
    }

    void GraphsWidget::doubleClickEvent(QMouseEvent * event)
    {
        QPoint bubbleAdjust(100,120);

        for (int i = 0; i < this->_xData.size(); ++i)
        {
            QRect nodeRect(this->_xAxis->resolve(this->_xData[i]) - 6, this->_yAxis->resolve(this->_yData[i]) - 6, 12, 12);
            //this->_view->scene()->addRect(nodeRect);
            if (nodeRect.contains(event->pos()))
            {
                this->activate(this->_labelData[i]);
            }
        }
    }


    void GraphsWidget::mouseReleaseEvent(QMouseEvent * event)
    {
        this->_view->scene()->removeItem(this->_bubblePixmap);
        this->_view->scene()->removeItem(this->_bubbleTitle);
    }

    /*
      void GraphsWidget::keyPressEvent(QKeyEvent * event)
      {
      if (event->modifiers() & Qt::ControlModifier)
      {
      this->_view->setDragMode(QGraphicsView::ScrollHandDrag);
      }
      }


      void GraphsWidget::keyReleaseEvent(QKeyEvent * event)
      {
      if (event->modifiers() & Qt::ControlModifier)
      {
      this->_view->setDragMode(QGraphicsView::RubberBandDrag);
      }
      else
      {
      this->_view->setDragMode(QGraphicsView::RubberBandDrag);
      }
      }

    */
    void GraphsWidget::xAxisRectChanged(QRectF )
    {
    }

    void GraphsWidget::yAxisRectChanged(QRectF )
    {
    }

    double GraphsWidget::zoom() const
    {
        return this->_zoom;
    }

    GraphsWidget::ZoomMode GraphsWidget::zoomMode() const
    {
        return this->_zoomMode;
    }

}
