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

#ifndef GRAFFITI_GRAPHTABLEVIEW_H
#define GRAFFITI_GRAPHTABLEVIEW_H

#include <graffiti/config.h>

#include <QTableView>

namespace Graffiti
{

    class TableWidget;

    class LIBGRAFFITI_API GraphTableView : public QTableView
    {
        Q_OBJECT
    public:
        GraphTableView(QWidget *parent, TableWidget *tableWidget);

        enum SeriesType { None = 0, XAxis = 1, YAxis = 2, Label = 3};

    signals:
        void typeChanged(QModelIndex modelIndex, Graffiti::GraphTableView::SeriesType type);
        void colourChanged(QModelIndex modelIndex, QColor colour);
        void checkChanged(QModelIndex modelIndex);
        void dataChanged();
        void dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);



    public slots:
        void cellClicked(const QModelIndex &index);
        void setCurrentColor(const QColor &);
        void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);



    protected:
        void mousePressEvent(QMouseEvent * event);
        TableWidget *_tableWidget;

    };

}

#endif
