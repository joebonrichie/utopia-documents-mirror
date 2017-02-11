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

#ifndef GRAFFITI_TABLEWIDGET_H
#define GRAFFITI_TABLEWIDGET_H

#include <graffiti/config.h>

class QTableView;
class QStandardItemModel;

#include <QWidget>
#include <QModelIndex>
#include <QDomDocument>

#include <graffiti/graphtableview.h>

namespace Graffiti
{

    class LIBGRAFFITI_API TableWidget : public QWidget
    {
        Q_OBJECT
    public:
        TableWidget(QStandardItemModel *model = NULL);

        enum DataSeriesOrientation { NoSeries = 0, ColumnSeries = 1, RowSeries = 2};
        enum GraphType { NoGraph = 0, ScatterPlot = 1, LineGraph = 2 };

        void setRowHeader(bool isHeader);
        void setColumnHeader(bool isHeader);
        bool rowHeader();
        bool columnHeader();

        void setColumnSeriesType(int columnIndex, GraphTableView::SeriesType type);
        void setRowSeriesType(int rowIndex, GraphTableView::SeriesType type);


        void setDataSeriesOrientation(DataSeriesOrientation orientation);
        DataSeriesOrientation dataSeriesOrientation();

        void setGraphType(GraphType graphType);
        GraphType graphType();


        bool rowIsNumeric(int row);
        bool columnIsNumeric(int row);

        int headerRowCount();
        static bool initModelFromNLMXML(QString filename, const QString & tableId, QStandardItemModel *model);
        static bool initModelFromNLMXMLData(QString data, const QString & tableId, QStandardItemModel *model);
        static bool initModelFromNLMDOM(QDomDocument dom, const QString & tableId, QStandardItemModel *model);
        static bool initModelFromNLMDOM(QDomElement table, QStandardItemModel *model);

        static bool initModelFromElsevierXMLData(QString data, const QString & tableId, QStandardItemModel *model);
        static bool initModelFromElsevierDOM(QDomDocument dom, const QString & tableId, QStandardItemModel *model);
        static bool initModelFromElsevierDOM(QDomElement table, QStandardItemModel *model);

        static bool initModelFromCortiData(QStandardItemModel *model);

        static bool initModelFromXML(QStandardItemModel * model, const QUrl & url, const QString & id = QString());
        static bool initModelFromXML(QStandardItemModel * model, QIODevice * io, const QString & id = QString());
        static bool initModelFromXML(QStandardItemModel * model, const QByteArray & xml, const QString & id = QString());
        static bool initModelFromXML(QStandardItemModel * model, const QString & xml, const QString & id = QString());
        static bool initModelFromXML(QStandardItemModel * model, QDomDocument document, const QString & id = QString());
        static bool initModelFromXML(QStandardItemModel * model, const QDomElement & table);

        void resizeColumnsToContents();
        int columns();

        QStandardItemModel * model() { return _model; }

    signals:
        void tableChanged(Graffiti::TableWidget::DataSeriesOrientation orientation, Graffiti::TableWidget::GraphType type, int headerRowCount);

    protected slots:
        void cellTypeChanged(QModelIndex index, Graffiti::GraphTableView::SeriesType type);
        void cellColourChanged(QModelIndex index, QColor colour);
        void cellCheckedChanged(QModelIndex index);
        void dataChanged();

    protected:
        QStandardItemModel *_model;
        GraphTableView *_view;

        bool _rowHeader;
        bool _columnHeader;

        int _headerRowCount;

        GraphType _graphType;
        DataSeriesOrientation _dataSeriesOrientation;
    };

}

#endif
