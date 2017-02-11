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

#include <papyro/sortfilterproxymodel.h>
#include <papyro/abstractfilter.h>

#include <QPointer>

namespace Athenaeum
{

    class SortFilterProxyModelPrivate
    {
    public:
        SortFilterProxyModelPrivate()
            : filter(0)
        {}

        QPointer< AbstractFilter > filter;
    }; // class SortFilterProxyModelPrivate




    SortFilterProxyModel::SortFilterProxyModel(QObject * parent)
        : QSortFilterProxyModel(parent), d(new SortFilterProxyModelPrivate)
    {}

    SortFilterProxyModel::~SortFilterProxyModel()
    {
        delete d;
    }

    AbstractFilter * SortFilterProxyModel::filter() const
    {
        return d->filter;
    }

    bool SortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex & /*source_parent*/) const
    {
        return source_column < 2;
    }

    bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
    {
        return !(d->filter && sourceModel()) || d->filter->accepts(sourceModel()->index(source_row, 0, source_parent));
    }

    void SortFilterProxyModel::setFilter(AbstractFilter * filter)
    {
        if (d->filter) {
            disconnect(d->filter.data(), SIGNAL(filterChanged()), this, SLOT(invalidate()));
        }
        d->filter = filter;
        if (d->filter) {
            connect(d->filter.data(), SIGNAL(filterChanged()), this, SLOT(invalidate()));
        }
        invalidateFilter();
    }

} // namespace Athenaeum
