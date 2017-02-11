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

#ifndef GRAFFITI_GRAPHSWIDGET_H
#define GRAFFITI_GRAPHSWIDGET_H

#include <graffiti/config.h>
#include <graffiti/tablewidget.h>

#include <QWidget>
#include <QGraphicsScene>

class QGraphicsView;
class QStandardItemModel;
class QGraphicsPixmapItem;
class QGraphicsSimpleTextItem;

namespace Graffiti
{

    class Axis;

    class LIBGRAFFITI_API GraphsWidget : public QWidget
    {
        Q_OBJECT

    public:
        typedef enum
        {
            FitToHeight,
            FitToWidth,
            FitToWindow,
            FitToGingPo = FitToWindow,
            CustomZoom
        } ZoomMode;

        // Constructors
        GraphsWidget(QWidget * parent = 0, Qt::WindowFlags f = 0, QStandardItemModel *model = NULL);
        // Destructor
        ~GraphsWidget();

        void setLinkTemplate(const QString & linkTemplate);
        void setZoomMode(ZoomMode mode);
        double zoom() const;
        ZoomMode zoomMode() const;

    signals:
        void resized();

    public slots:
        // AbstractWidget Interface

        void setZoom(double zoom);


        void dataChanged(Graffiti::TableWidget::DataSeriesOrientation orientation, Graffiti::TableWidget::GraphType type, int headerRowCount);

    protected:
        void resizeEvent(QResizeEvent * event);
        void wheelEvent(QWheelEvent * event);
        void mousePressEvent(QMouseEvent * event);
        void doubleClickEvent(QMouseEvent * event);

        void mouseReleaseEvent(QMouseEvent * event);

        //              void keyPressEvent(QKeyEvent * event);
        //              void keyReleaseEvent(QKeyEvent * event);


        bool eventFilter(QObject * obj, QEvent * event);

        void activate(QString name);

        QGraphicsPixmapItem *_bubblePixmap;
        QGraphicsSimpleTextItem *_bubbleTitle;


    protected slots:
        void xAxisRectChanged(QRectF rect);
        void yAxisRectChanged(QRectF rect);

    private:
        // Graphics View
        QGraphicsView * _view;
        QGraphicsScene * _scene;

        QStandardItemModel *_model;

        int _bubbleIndex;

        void initialise();

        Axis * _xAxis;
        Axis * _yAxis;

        double _zoom;
        ZoomMode _zoomMode;

        void resizeScene();

        // Hard wired - REMOVE FIXME
        double _x[21];
        double _y[21];

        QList< double > _xData;
        QList< double > _yData;
        QList< QString > _labelData;
        QString _linkTemplate;

        TableWidget::DataSeriesOrientation _oldDataSeries;
        TableWidget::GraphType _oldType;
        int _headerRowCount;

    }; // class GraphsWidget

}

#endif
