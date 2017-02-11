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

#include <graffiti/tablewidget.h>
#include <graffiti/entrydelegate.h>

#include <QWidget>
#include <QStandardItemModel>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRegExp>

#include <QDomDocument>
#include <QFile>
#include <QDebug>


#include <graffiti/graphtableview.h>

namespace Graffiti
{

    TableWidget::TableWidget(QStandardItemModel *model) : QWidget(), _model(model), _dataSeriesOrientation(NoSeries)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);

        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);


        this->_view = new GraphTableView(this, this);
        this->_view->setAlternatingRowColors(false);
        this->_view->setModel(this->_model);
        this->_view->setItemDelegate(new EntryDelegate(this));
        this->_view->setShowGrid(false);
        this->_view->setFrameStyle(QFrame::NoFrame);
        layout->addWidget(this->_view);

        for (int i = 0; i < this->_model->rowCount() ; ++i)
        {
            this->_view->setRowHeight(i, 22);
        };

        this->_headerRowCount = 0;

        for (int row = 0; row < this->_model->rowCount() ; ++row)
        {
            if (this->_model->data(this->_model->index(row,0),Qt::UserRole + 3).toBool())
            {
                //qDebug() << "Things";
                ++this->_headerRowCount;
            }

            for (int column = 0 ; column < this->_model->columnCount() ; ++column)
            {
                this->_model->setData(this->_model->index(row,column), GraphTableView::None, Qt::UserRole);
                this->_model->setData(this->_model->index(row,column), QColor(Qt::red), Qt::UserRole + 1);
                this->_model->setData(this->_model->index(row,column), false, Qt::UserRole + 2);
            }
        }

        this->_view->resizeColumnsToContents();
        this->_view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        this->_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

        this->_view->show();
        this->_view->raise();

        connect(this->_view, SIGNAL(typeChanged(QModelIndex, Graffiti::GraphTableView::SeriesType)), this, SLOT(cellTypeChanged(QModelIndex, Graffiti::GraphTableView::SeriesType)));
        connect(this->_view, SIGNAL(colourChanged(QModelIndex, QColor)), this, SLOT(cellColourChanged(QModelIndex, QColor)));
        connect(this->_view, SIGNAL(checkChanged(QModelIndex)), this, SLOT(cellCheckedChanged(QModelIndex)));
        connect(this->_view, SIGNAL(dataChanged()), this, SLOT(dataChanged()));

    }

    void TableWidget::resizeColumnsToContents()
    {
        this->_view->resizeColumnsToContents();
    }

    int TableWidget::columns()
    {
        return this->_model->columnCount();
    }

    void TableWidget::setRowHeader(bool isHeader)
    {
        this->_rowHeader = isHeader;

        //qDebug() << "NUMBER OF HEADER ROWS IS " << this->headerRowCount();
        if (this->_rowHeader && this->_dataSeriesOrientation == ColumnSeries)
        {
            this->_view->setRowHeight(this->headerRowCount() - 1, 44);
        }
        else
        {
            this->_view->setRowHeight(this->headerRowCount() - 1, 22);
        }
    }

    int TableWidget::headerRowCount()
    {
        return this->_headerRowCount;
    }

    bool TableWidget::columnIsNumeric(int column)
    {
        bool isNumeric = true;
        for (int row = this->headerRowCount(); row < this->_model->rowCount() ; ++row)
        {
            QString data = this->_model->data(this->_model->index(row, column)).toString();

            //qDebug() << "Data is " << data;

            if (data.contains(QChar(0x00B1)))
            {
                //qDebug() << "Got a plusminus";
                int index = data.indexOf(QChar(0x00B1));
                data = data.left(index);
                //qDebug() << "Stripped to " << data;
            }

            if (data.contains(QRegExp("\\s")))
            {
                //qDebug() << "Got whitespace";
                int index = data.indexOf(QRegExp("\\s"));
                data = data.left(index);
                //qDebug() << "Stripped to " << data;
            }

            bool canConvertToDouble = false;
            data.toDouble(&canConvertToDouble);
            if (!canConvertToDouble)
            {
                qDebug() << "Failed to convert";
                isNumeric = false;
            }
        }

        return isNumeric;
    }

    bool TableWidget::rowIsNumeric(int row)
    {
        bool isNumeric = true;
        for (int column = 1; column < this->_model->columnCount() ; ++column)
        {
            QString data = this->_model->data(this->_model->index(row, column)).toString();
            bool canConvertToDouble = false;
            data.toDouble(&canConvertToDouble);
            if (!canConvertToDouble)
            {
                isNumeric = false;
            }
        }

        return isNumeric;
    }



    void TableWidget::cellTypeChanged(QModelIndex index, GraphTableView::SeriesType type)
    {

        bool okToChange = true;

        if (this->_dataSeriesOrientation == TableWidget::ColumnSeries)
        {
            if ((type == GraphTableView::XAxis || type == GraphTableView::YAxis) && !this->columnIsNumeric(index.column()))
            {
                QMessageBox msgBox;
                msgBox.setText("This column can not be plotted as a graph because is contains non-numerical data");
                msgBox.exec();
                okToChange = false;
            }
        }
        else if (this->_dataSeriesOrientation == TableWidget::RowSeries)
        {
            if ((type == GraphTableView::XAxis || type == GraphTableView::YAxis) && !this->rowIsNumeric(index.row()))
            {
                QMessageBox msgBox;
                msgBox.setText("This row can not be plotted as a graph because is contains non-numerical data");
                msgBox.exec();
                okToChange = false;
            }
        }

        if (okToChange)
        {
            // if another row / column already has this SeriesType, set it to none
            if (this->_dataSeriesOrientation == TableWidget::ColumnSeries)
            {
                for (int column = 0 ; column < this->_model->columnCount(); ++column)
                {
                    if (this->_model->data(this->_model->index(this->headerRowCount() - 1,column), Qt::UserRole).toInt() == type)
                    {
                        this->_model->setData(this->_model->index(this->headerRowCount() - 1,column),  GraphTableView::None, Qt::UserRole);
                    }
                }
            }
            else
            {
                for (int row = 0 ; row < this->_model->rowCount(); ++row)
                {
                    if (this->_model->data(this->_model->index(row,0), Qt::UserRole).toInt() == type)
                    {
                        this->_model->setData(this->_model->index(row,0),  GraphTableView::None, Qt::UserRole);
                    }
                }
            }

            this->_model->setData(index, type, Qt::UserRole);

            emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());
        }
    }

    void TableWidget::cellColourChanged(QModelIndex index, QColor colour)
    {
        this->_model->setData(index, colour, Qt::UserRole + 1);
        emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());
    }

    void TableWidget::cellCheckedChanged(QModelIndex index)
    {
        bool current = this->_model->data(index, Qt::UserRole + 2).toBool();
        this->_model->setData(index, !current, Qt::UserRole + 2);
        emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());
    }

    void TableWidget::dataChanged()
    {
        emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());
    }



    void TableWidget::setColumnHeader(bool isHeader)
    {
        //      for (int rowIndex = 0; rowIndex < this->_model->rowCount(); ++rowIndex)
        //      {
        //              this->_model->setData(this->_model->index(rowIndex, 0), true, Qt::UserRole);
        //      }

        this->_columnHeader = isHeader;
    }

    bool TableWidget::rowHeader()
    {
        return this->_rowHeader;
    }

    bool TableWidget::columnHeader()
    {
        return this->_columnHeader;
    }

    void TableWidget::setDataSeriesOrientation(DataSeriesOrientation orientation)
    {
        this->_dataSeriesOrientation = orientation;
        if (this->_rowHeader && this->_dataSeriesOrientation == ColumnSeries)
        {
            this->_view->setRowHeight(this->headerRowCount() - 1, 44);
        }
        else
        {
            this->_view->setRowHeight(this->headerRowCount() - 1, 22);
        }
    }

    TableWidget::DataSeriesOrientation TableWidget::dataSeriesOrientation()
    {
        return this->_dataSeriesOrientation;
    }

    void TableWidget::setGraphType(TableWidget::GraphType graphType)
    {
        this->_graphType = graphType;
    }

    TableWidget::GraphType TableWidget::graphType()
    {
        return this->_graphType;
    }



    void TableWidget::setColumnSeriesType(int columnIndex, GraphTableView::SeriesType type)
    {
        this->_model->setData(this->_model->index(this->headerRowCount() - 1, columnIndex),  type, Qt::UserRole);
        emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());

    }

    void TableWidget::setRowSeriesType(int rowIndex, GraphTableView::SeriesType type)
    {
        this->_model->setData(this->_model->index(rowIndex, 0),  type, Qt::UserRole);
        emit tableChanged(this->_dataSeriesOrientation, this->_graphType, this->headerRowCount());
    }


    bool TableWidget::initModelFromNLMXMLData(QString data, const QString & tableId, QStandardItemModel *model)
    {
        QDomDocument doc("mydocument");
        QString errorString;
        int errorLine = 0;
        int errorColumn = 0;
        if (!doc.setContent(data, &errorString, &errorLine, &errorColumn))
        {

            qDebug() << "ERROR: Read NLM as XML" << errorString << errorLine << errorColumn;
            return false;
        }
        return initModelFromNLMDOM(doc, tableId, model);
    }

    bool TableWidget::initModelFromNLMXML(QString filename, const QString & tableId, QStandardItemModel *model)
    {
        QDomDocument doc("mydocument");
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "ERROR: Couldn't open NLM file";
            return false;
        }
        if (!doc.setContent(&file))
        {
            qDebug() << "ERROR: Read NLM as XML";
            file.close();
            return false;
        }
        file.close();

        return initModelFromNLMDOM(doc, tableId, model);
    }

    bool TableWidget::initModelFromNLMDOM(QDomDocument doc, const QString & tableId, QStandardItemModel *model)
    {
        if (tableId.isEmpty())
        {
            QDomElement table = doc.documentElement();
            if (!table.isNull())
            {
                return initModelFromNLMDOM(table, model);
            }
            else
            {
                qDebug() << "ERROR: Couldn't parse <table>";
                return false;
            }
        }
        else
        {
            QDomElement docElem = doc.documentElement();

            QDomNodeList tableWrapList = docElem.elementsByTagName("table-wrap");

            bool found = false;
            QDomElement tableWrapper;
            for (int tableIndex = 0; tableIndex < tableWrapList.count(); ++tableIndex)
            {
                tableWrapper = tableWrapList.at(tableIndex).toElement();
                if (tableWrapper.hasAttribute("id"))
                {
                    //qDebug() << "Found table with id = " << tableWrapper.attribute("id");
                    if (tableWrapper.attribute("id") == tableId)
                    {
                        //qDebug() << "Found the table we need";
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
            {
                qDebug() << "ERROR : Failed to find table with id of " << tableId;
                return false;
            }

            QDomElement table = tableWrapper.firstChildElement("table");
            if (!table.isNull())
            {
                return initModelFromNLMDOM(table, model);
            }
            else
            {
                qDebug() << "ERROR: Couldn't find <table>";
                return false;
            }
        }
    }


    bool TableWidget::initModelFromNLMDOM(QDomElement table, QStandardItemModel *model)
    {
        int maxColumnCount = 0;
        QList<QStringList> tableContents;
        int headerCount = 0;
        int bodyCount = 0;

        QDomElement tableHeader = table.firstChildElement("thead");
        if (!tableHeader.isNull())
        {
            //qDebug() << "Found <thead>";
            QDomNodeList rownodes = tableHeader.elementsByTagName("tr");
            for (int row = 0; row < rownodes.count(); ++row)
            {
                tableContents.insert(row,QStringList());
                QDomElement tablerow = rownodes.at(row).toElement();
                if (!tablerow.isNull())
                {
                    //qDebug() << "Found header row";
                    ++headerCount;
                    QDomNodeList headervaluelist = tablerow.childNodes();
                    for (int headervalueindex = 0 ; headervalueindex < headervaluelist.count() ; ++headervalueindex)
                    {
                        QDomElement headervalue= headervaluelist.at(headervalueindex).toElement();
                        if (headervalue.tagName() == "th" || headervalue.tagName() == "td") {
                            //qDebug() << "Found header cell " << headervalue.text();
                            tableContents[row].append(headervalue.text().trimmed());

                            if (headervalue.hasAttribute("colspan"))
                            {
                                //qDebug() << "Multiple columns spanned";
                                double colspan = headervalue.attribute("colspan").toDouble();
                                if (colspan>1)
                                {
                                    // add in empty columns to fill the extra spanned columns
                                    for (int i = 0; i < colspan - 1; ++i)
                                    {
                                        //qDebug() << "Adding a span";
                                        tableContents[row].append("");
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }
        else
        {
            qDebug() << "ERROR: Couldn't find <thead>";
            //return false;
        }

        QDomElement tableBody = table.firstChildElement("tbody");
        if (!tableBody.isNull())
        {
            //qDebug() << "Found <tbody>";
            QDomNodeList rownodes = tableBody.elementsByTagName("tr");
            for (int row = 0; row < rownodes.count(); ++row)
            {
                tableContents.insert(row + headerCount,QStringList());
                QDomElement tablerow = rownodes.at(row).toElement();
                if (!tablerow.isNull())
                {
                    //qDebug() << "Found body row";
                    ++bodyCount;
                    QDomNodeList bodyvaluelist = tablerow.childNodes();
                    for (int bodyvalueindex = 0 ; bodyvalueindex < bodyvaluelist.count() ; ++bodyvalueindex)
                    {
                        QDomElement bodyvalue= bodyvaluelist.at(bodyvalueindex).toElement();
                        if (bodyvalue.tagName() == "th" || bodyvalue.tagName() == "td") {
                            //qDebug() << "Found body cell " << bodyvalue.text();
                            if (bodyvalueindex < 0) {
                                tableContents[row + headerCount].append(bodyvalue.text().trimmed().remove(QRegExp("[^0-9.,e+-]+$")));
                            } else {
                                tableContents[row + headerCount].append(bodyvalue.text().trimmed());
                            }

                            if (bodyvalue.hasAttribute("colspan"))
                            {
                                //qDebug() << "Multiple columns spanned";
                                double colspan = bodyvalue.attribute("colspan").toDouble();
                                if (colspan>1)
                                {
                                    // add in empty columns to fill the extra spanned columns
                                    for (int i = 0; i < colspan - 1; ++i)
                                    {
                                        //qDebug() << "Adding a span";
                                        tableContents[row + headerCount].append("");
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }
        else
        {
            qDebug() << "ERROR: Couldn't find <tbody>";
            return false;
        }

        maxColumnCount = 0;
        if (tableContents.count()>0)
        {
            maxColumnCount = tableContents.at(0).count();
            //qDebug() << "columnCount = " << maxColumnCount;
            for (int row = 0 ; row < tableContents.count() ; ++row)
            {
                if (tableContents.at(row).count() > maxColumnCount)
                {
                    maxColumnCount = tableContents.at(row).count();
                }
            }

            //qDebug() << "Max column count is " << maxColumnCount;
        }
        else
        {
            qDebug() << "ERROR: Table has accumulated no contents";
            return false;
        }

        if (tableContents.count() != headerCount + bodyCount)
        {
            qDebug() << "ERROR: table rows do not match header + body rows";
        }

        model->clear();

        // work out how many valid rows there are
        int rowInTable = 0;
        for (int row=0; row<tableContents.count(); ++row)
        {
            if (tableContents.at(row).count() == maxColumnCount)
            {
                rowInTable++;
            }
            else
            {
                qDebug() << "WARNING : Row " << row << " doesn't have the same number of columns as the max; rejecting it";
            }
        }


        model->setRowCount(rowInTable);
        model->setColumnCount(maxColumnCount);
        rowInTable = 0;
        for (int row=0; row<tableContents.count(); ++row)
        {
        	if (tableContents.at(row).count() == maxColumnCount)
        	{
				for (int column = 0; column < tableContents.at(row).count(); ++column)
				{
					//qDebug() << "adding " << tableContents.at(row).at(column);
					QString rowItem(tableContents.at(row).at(column));
					if (QRegExp("[\\d,\\s.-]+").exactMatch(rowItem))
					{
					    rowItem = rowItem.replace(",", "");
					}
					model->setData(model->index(rowInTable, column), rowItem, Qt::EditRole);

					// Set the user role to true for header cells / rows
					if (row < headerCount)
					{
						model->setData(model->index(rowInTable, column), true, Qt::UserRole + 3);
					}
					else
					{
						model->setData(model->index(rowInTable, column), false, Qt::UserRole + 3);
					}
				}
				rowInTable++;
            }
            else
            {
                //qDebug() << "Row " << row << " doesn't have the same number of columns as the max; ignoring it";
            }
        }

        return true;
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, const QUrl & url, const QString & id)
    {
        return false;
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, QIODevice * io, const QString & id)
    {
        QDomDocument doc;
        return doc.setContent(io) && initModelFromXML(model, doc, id);
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, const QByteArray & xml, const QString & id)
    {
        QDomDocument doc;
        return doc.setContent(xml) && initModelFromXML(model, doc, id);
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, const QString & xml, const QString & id)
    {
        QDomDocument doc;
        return doc.setContent(xml) && initModelFromXML(model, doc, id);
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, QDomDocument document, const QString & id)
    {
        return initModelFromXML(model, id.isEmpty() ? document.documentElement() : document.elementById(id));
    }

    bool TableWidget::initModelFromXML(QStandardItemModel * model, const QDomElement & table)
    {
        // Ensure the element isn't null, and is of type "table"
        if (table.isNull() || table.tagName() != "table") { return false; }

        int rowIndex = 0;
        for (QDomElement group(table.firstChildElement());
             !group.isNull(); group = group.nextSiblingElement()) {
            const bool isHead = group.tagName() == "thead";
            const bool isBody = group.tagName() == "tbody";
            //const bool isFoot = group.tagName() == "tfoot";
            if (!isHead && !isBody) { continue; }
            for (QDomElement row(group.firstChildElement());
                 !row.isNull(); row = row.nextSiblingElement()) {
                int columnIndex = 0;
                for (QDomElement cell(row.firstChildElement());
                     !cell.isNull(); cell = cell.nextSiblingElement()) {
                    const bool isTd = cell.tagName() == "td";
                    const bool isTh = cell.tagName() == "th";
                    if (!isTd && !isTh) { continue; }
                    const int colspan = cell.attribute("colspan", "1").toInt();
                    const int rowspan = cell.attribute("rowspan", "1").toInt();
                    while (model->item(rowIndex, columnIndex) != 0) { ++columnIndex; } // Find next empty cell
                    for (int x = columnIndex; x < (columnIndex + colspan); ++x) {
                        for (int y = rowIndex; y < (rowIndex + rowspan); ++y) {
                            QStandardItem * item = new QStandardItem;
                            if (x == columnIndex && y == rowIndex) {
                                QString text(cell.text().trimmed());
                                // Remove commas from numeric-looking data. FIXME: a bit blunt
                                static const QRegExp numericish("[\\d,\\s.-]+");
                                if (numericish.exactMatch(text)) {
                                    text.replace(",", "");
                                }
                                item->setData(text, Qt::EditRole);
                            }
                            item->setData(isHead, Qt::UserRole + 3);
                            model->setItem(y, x, item);
                        }
                    }
                }
                ++rowIndex;
            }
        }
        return true;
    }

    bool TableWidget::initModelFromElsevierXMLData(QString data, const QString & tableId, QStandardItemModel *model)
    {
		QDomDocument doc("mydocument");
		QString errorString;
		int errorLine = 0;
		int errorColumn = 0;
		if (!doc.setContent(data, &errorString, &errorLine, &errorColumn))
		{

		 qDebug() << "ERROR: Read Elsevier as XML" << errorString << errorLine << errorColumn;
		 return false;
		}
        return initModelFromElsevierDOM(doc, tableId, model);
    }

    bool TableWidget::initModelFromElsevierDOM(QDomDocument doc, const QString & tableId, QStandardItemModel *model)
    {
        if (tableId.isEmpty())
        {
            QDomElement table = doc.documentElement();
            if (!table.isNull())
            {
                return initModelFromElsevierDOM(table, model);
            }
            else
            {
                qDebug() << "ERROR: Couldn't parse <table>";
                return false;
            }
        }
        else
        {
            QDomElement docElem = doc.documentElement();

            QDomNodeList tableWrapList = docElem.elementsByTagName("table");

            bool found = false;
            QDomElement tableWrapper;
            for (int tableIndex = 0; tableIndex < tableWrapList.count(); ++tableIndex)
            {
                tableWrapper = tableWrapList.at(tableIndex).toElement();
                if (tableWrapper.hasAttribute("id"))
                {
                    //qDebug() << "Found table with id = " << tableWrapper.attribute("id");
                    if (tableWrapper.attribute("id") == tableId)
                    {
                        //qDebug() << "Found the table we need";
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
            {
                qDebug() << "ERROR : Failed to find table with id of " << tableId;
                return false;
            }

            QDomElement table = tableWrapper.firstChildElement("tgroup");
            if (!table.isNull())
            {
                return initModelFromElsevierDOM(table, model);
            }
            else
            {
                qDebug() << "ERROR: Couldn't find <table>";
                return false;
            }
		}
	}


    bool TableWidget::initModelFromElsevierDOM(QDomElement table, QStandardItemModel *model)
    {
		int maxColumnCount = 0;
		QList<QStringList> tableContents;
		int headerCount = 0;
		int bodyCount = 0;

        QDomElement tableHeader = table.firstChildElement("thead");
        if (!tableHeader.isNull())
        {
            //qDebug() << "Found <thead>";
            QDomNodeList rownodes = tableHeader.elementsByTagName("row");
            for (int row = 0; row < rownodes.count(); ++row)
            {
                tableContents.insert(row,QStringList());
                QDomElement tablerow = rownodes.at(row).toElement();
                if (!tablerow.isNull())
                {
                    //qDebug() << "Found header row";
                    ++headerCount;
                    QDomNodeList headervaluelist = tablerow.elementsByTagName("entry");
                    for (int headervalueindex = 0 ; headervalueindex < headervaluelist.count() ; ++headervalueindex)
                    {
                        QDomElement headervalue= headervaluelist.at(headervalueindex).toElement();
                        //qDebug() << "Found header cell " << headervalue.text();
                        tableContents[row].append(headervalue.text());

                        if (headervalue.hasAttribute("colspan"))
                        {
                            //qDebug() << "Multiple columns spanned";
                            double colspan = headervalue.attribute("colspan").toDouble();
                            if (colspan>1)
                            {
                                // add in empty columns to fill the extra spanned columns
                                for (int i = 0; i < colspan - 1; ++i)
                                {
                                    //qDebug() << "Adding a span";
                                    tableContents[row].append("");
                                }
                            }
                        }

                    }
                }
            }
        }
        else
        {
            qDebug() << "ERROR: Couldn't find <thead>";
            return false;
        }

        QDomElement tableBody = table.firstChildElement("tbody");
        if (!tableBody.isNull())
        {
            //qDebug() << "Found <tbody>";
            QDomNodeList rownodes = tableBody.elementsByTagName("row");
            for (int row = 0; row < rownodes.count(); ++row)
            {
                tableContents.insert(row + headerCount,QStringList());
                QDomElement tablerow = rownodes.at(row).toElement();
                if (!tablerow.isNull())
                {
                    //qDebug() << "Found body row";
                    ++bodyCount;
                    QDomNodeList bodyvaluelist = tablerow.elementsByTagName("entry");
                    for (int bodyvalueindex = 0 ; bodyvalueindex < bodyvaluelist.count() ; ++bodyvalueindex)
                    {
                        QDomElement bodyvalue= bodyvaluelist.at(bodyvalueindex).toElement();
                        //qDebug() << "Found body cell " << bodyvalue.text();
                        tableContents[row + headerCount].append(bodyvalue.text().trimmed());

                        if (bodyvalue.hasAttribute("colspan"))
                        {
                            //qDebug() << "Multiple columns spanned";
                            double colspan = bodyvalue.attribute("colspan").toDouble();
                            if (colspan>1)
                            {
                                // add in empty columns to fill the extra spanned columns
                                for (int i = 0; i < colspan - 1; ++i)
                                {
                                    //qDebug() << "Adding a span";
                                    tableContents[row + headerCount].append("");
                                }
                            }
                        }

                    }
                }
            }
        }
        else
        {
            qDebug() << "ERROR: Couldn't find <tbody>";
            return false;
        }

		maxColumnCount = 0;
		if (tableContents.count()>0)
		{
			maxColumnCount = tableContents.at(0).count();
			//qDebug() << "columnCount = " << maxColumnCount;
			for (int row = 0 ; row < tableContents.count() ; ++row)
			{
				if (tableContents.at(row).count() > maxColumnCount)
				{
					maxColumnCount = tableContents.at(row).count();
				}
			}

			//qDebug() << "Max column count is " << maxColumnCount;
		}
		else
		{
			qDebug() << "ERROR: Table has accumulated no contents";
			return false;
		}

        if (tableContents.count() != headerCount + bodyCount)
        {
        	qDebug() << "ERROR: table rows do not match header + body rows";
        }

        model->clear();

		// work out how many valid rows there are
        int rowInTable = 0;
        for (int row=0; row<tableContents.count(); ++row)
        {
        	if (tableContents.at(row).count() == maxColumnCount)
        	{
				rowInTable++;
            }
            else
            {
            	qDebug() << "WARNING : Row " << row << " doesn't have the same number of columns as the max; rejecting it";
            }
        }


        model->setRowCount(rowInTable);
        model->setColumnCount(maxColumnCount);
        rowInTable = 0;
        for (int row=0; row<tableContents.count(); ++row)
        {
        	if (tableContents.at(row).count() == maxColumnCount)
        	{
				for (int column = 0; column < tableContents.at(row).count(); ++column)
				{
					//qDebug() << "adding " << tableContents.at(row).at(column);
					QString rowItem(tableContents.at(row).at(column));
					if (QRegExp("[\\d,\\s.-]+").exactMatch(rowItem))
					{
					    rowItem = rowItem.replace(",", "");
					}
					model->setData(model->index(rowInTable, column), rowItem, Qt::EditRole);

					// Set the user role to true for header cells / rows
					if (row < headerCount)
					{
						model->setData(model->index(rowInTable, column), true, Qt::UserRole + 3);
					}
					else
					{
						model->setData(model->index(rowInTable, column), false, Qt::UserRole + 3);
					}
				}
				rowInTable++;
            }
            else
            {
            	//qDebug() << "Row " << row << " doesn't have the same number of columns as the max; ignoring it";
            }
        }

        return true;
    }

    bool TableWidget::initModelFromCortiData(QStandardItemModel *model)
    {
        QStringList cortilines;
/*
        cortilines << "Compound A.M.Papp R% Fa% Caco-2b-Papp PAMPA-Papp LogKo/w LogD"
                   << "Chlorothiazide 0.86±0.04 99.1 13 0.015 0.13 -0.24 -0.05"
                   <<   "Aciclovir 0.91±0.02 99.9 21 0.025 0.00 -1.74 -1.86"
                   <<   "Nadolol 1.37±0.03 99.5 32 0.388 0.00 0.71 0.68"
                   <<   "Alpha-Methyl+dopa 0.32±0.01 97.1 41 0.015 0.00 -1.80 -1.80"
                   <<   "Atenolol 2.09±0.10 98.6 52 0.020 0.00 0.16 -1.29"
                   << "Ranitidine 2.15±0.03 99.9 55 0.049 0.05 0.27 -0.29"
                   <<   "Metformin 2.27±0.20 97.3 55 0.550 0.02 -1.43 -1.22"
                   <<   "Furosemide 2.75±0.02 99.0 60 0.012 0.06 2.29 -0.69"
                   <<   "Hydrochlorothiazide 3.10±+0.05 98.3 70 0.051 0.00 -0.07 -0.12"
                   <<   "Chloramphenicol 3.97±0.01 99.3 90 2.06 0.17 1.14 1.14"
                   <<   "Hydrocortisone 4.28±0.07 99.8 91 1.40 0.34 1.61 1.55"
                   <<   "Pindolol 3.74±0.07 99.2 92 1.67 0.49 1.75 0.19"
                   <<   "Propranolol 3.97±0.08 99.8 93 4.19 2.35 1.25 1.25"
                   <<   "Metoprolol 4.81±0.08 99.6 95 2.37 0.35 1.88 -0.16"
                   <<   "Theophylline 4.05±0.06 99.1 97 2.52 0.48 -0.25 -0.05"
                   <<   "Trimethoprim 4.55±0.09 99.8 97 8.30 0.50 0.91 0.74"
                   <<   "Naproxen 4.88±0.02 98.9 98 3.95 1.06 3.18 0.23"
                   <<   "Verapamil 4.16±0.03 97.5 98 1.58 0.74 3.79 2.66"
                   <<   "Antipyrine 4.91±0.03 97.8 100 2.82 1.32 0.38 0.34"
                   <<   "Ketoprofen 4.27±0.08 99.1 100 2.01 1.67 3.12 -1.51"
                   <<   "Caffeine 4.11±0.08 99.3 100 3.08 1.08 -0.07 0.02";
*/
        model->clear();
        model->setRowCount(22);
        model->setColumnCount(8);


        for (int row=0; row<cortilines.count(); ++row)
        {
            QStringList fields = cortilines.at(row).split(' ');
            for (int column = 0; column < fields.count(); ++column)
            {
                model->setData(model->index(row, column), fields.at(column), Qt::EditRole);
                if (row == 0)
                {
                    model->setData(model->index(row, column), true, Qt::UserRole + 3);
                }
            }
        }

        return true;
    }

}
