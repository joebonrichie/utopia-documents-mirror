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

#include <utopia2/auth/qt/servicechooser_p.h>
#include <utopia2/auth/qt/servicechooser.h>
#include <utopia2/auth/servicemanagermodel.h>
#include <utopia2/auth/servicemanagerfilterproxymodel.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QSignalMapper>

#include <QDebug>

Q_DECLARE_METATYPE(QPointer< Kend::Service >);

namespace Kend
{

    ServiceChooserPrivate::ServiceChooserPrivate(ServiceChooser * chooser, ServiceManager::Filters filters)
        : QObject(chooser), manager(ServiceManager::instance()), invalidMessage("<em>No valid service</em>"), format("%1")
    {
        connect(this, SIGNAL(chosenServiceChanged(Service *)), chooser, SIGNAL(chosenServiceChanged(Service *)));
        connect(this, SIGNAL(validityChanged(bool)), chooser, SIGNAL(validityChanged(bool)));

        model = new ServiceManagerModel(this);
        proxy = new ServiceManagerFilterProxyModel(filters, this);
        proxy->setSourceModel(model);
        connect(proxy, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(onRowsInserted(const QModelIndex &, int, int)));
        connect(proxy, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(onRowsAboutToBeRemoved(const QModelIndex &, int, int)));
        selection = new QItemSelectionModel(proxy, proxy);
        connect(selection, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

        QHBoxLayout * hBoxLayout = new QHBoxLayout(chooser);
        chosenServiceName = new QLabel;
        chosenServiceName->setAlignment(Qt::AlignRight);
        hBoxLayout->addWidget(chosenServiceName, 1);

        button = new QPushButton("list");
        button->setVisible(proxy->rowCount() > 1);
        hBoxLayout->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onButtonPressed()));

        selectRow(0);
        setChosenServiceName();
    }

    void ServiceChooserPrivate::onButtonPressed()
    {
        QMenu menu;
        QSignalMapper mapper;
        connect(&mapper, SIGNAL(mapped(const QString &)), this, SLOT(setCurrentServiceUuid(const QString &)));
        for (int r = 0; r < proxy->rowCount(); ++r) {
            QModelIndex index = proxy->index(r, 0);
            QString description = proxy->data(index).toString();
            QAction * action = menu.addAction(description, &mapper, SLOT(map()));
            mapper.setMapping(action, proxy->data(index, ServiceManagerModel::UuidRole).toString());
        }
        menu.exec(button->mapToGlobal(button->rect().bottomLeft()));
    }

    void ServiceChooserPrivate::onRowsInserted(const QModelIndex & parent, int start, int end)
    {
        if (selectedRow() < 0) {
            selectRow(0);
        }
        button->setVisible(proxy->rowCount() > 1);
    }

    void ServiceChooserPrivate::onRowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
    {
        QModelIndex selected = selection->currentIndex();
        if (selected.isValid() && selected.row() >= start && selected.row() <= end) {
            if (proxy->rowCount() > end + 1) {
                selectRow(end + 1);
            } else if (start > 0) {
                selectRow(start - 1);
            }
        }
        button->setVisible(proxy->rowCount() > 1);
    }

    void ServiceChooserPrivate::onSelectionChanged(const QItemSelection & current, const QItemSelection &)
    {
        QPointer< Service > newSelectedService = proxy->data(proxy->index(selectedRow(), 0), ServiceManagerModel::ServiceRole).value< QPointer< Service > >();
        if ((bool) selectedService == !newSelectedService) {
            emit validityChanged(newSelectedService);
        }
        selectedService = newSelectedService;
        setChosenServiceName();
        emit chosenServiceChanged(selectedService);
    }

    int ServiceChooserPrivate::selectedRow() const
    {
        QModelIndexList indices = selection->selection().indexes();
        if (!indices.isEmpty()) {
            return indices.first().row();
        }
        return -1;
    }

    void ServiceChooserPrivate::selectRow(int row)
    {
        selection->select(proxy->index(row, 0), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    }

    void ServiceChooserPrivate::setChosenServiceName()
    {
        QModelIndex index(proxy->index(selectedRow(), 0));
        if (index.isValid()) {
            chosenServiceName->setText(format.arg(proxy->data(index).toString()));
        } else {
            chosenServiceName->setText(invalidMessage);
        }
    }

    void ServiceChooserPrivate::setCurrentServiceUuid(const QString & uuid)
    {
        for (int r = 0; r < proxy->rowCount(); ++r) {
            QModelIndex index = proxy->index(r, 0);
            if (uuid == proxy->data(index, ServiceManagerModel::UuidRole).toString()) {
                selection->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            }
        }
    }





    ServiceChooser::ServiceChooser(ServiceManager::Filters filters, QWidget * parent)
        : QFrame(parent), d(new ServiceChooserPrivate(this, filters))
    {}

    ServiceManager::Filters ServiceChooser::filters() const
    {
        return d->proxy->filters();
    }

    QString ServiceChooser::format() const
    {
        return d->format;
    }

    QString ServiceChooser::invalidMessage() const
    {
        return d->invalidMessage;
    }

    QPointer< Service > ServiceChooser::selectedService() const
    {
        return d->selectedService;
    }

    void ServiceChooser::setFilters(ServiceManager::Filters filters)
    {
        d->proxy->setFilters(filters);
    }

    void ServiceChooser::setFormat(const QString & format)
    {
        d->format = format;
    }

    void ServiceChooser::setInvalidMessage(const QString & message)
    {
        d->invalidMessage = message;
    }

}
