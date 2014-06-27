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

#include <athenaeum/abstractbibliographicmodel.h>
#include <athenaeum/bibliographicmimedata_p.h>

#include <QSortFilterProxyModel>
#include <QDebug>

namespace Athenaeum
{

    static const QAbstractItemModel * origin(const QAbstractItemModel * model)
    {
        if (const QSortFilterProxyModel * proxy = qobject_cast< const QSortFilterProxyModel * >(model)) {
            return origin(proxy->sourceModel());
        } else {
            return model;
        }
    }

    bool AbstractBibliographicModel::acceptsDrop(const QMimeData * mime) const
    {
        // Check in case it's an internal move from/to the same model
        if (mime->hasFormat(_INTERNAL_MIMETYPE_BIBLIOGRAPHICITEMS)) {
            const BibliographicMimeData * bibData = qobject_cast< const BibliographicMimeData * >(mime);
            if (bibData && !bibData->indexes().isEmpty() && origin(bibData->indexes().first().model()) == origin(index(0, 0).model())) {
                return false;
            }
        }

        foreach (const QString & type, mimeTypes()) {
            if (mime->hasFormat(type)) {
                return true;
            }
        }
        return false;
    }

    QMimeData * AbstractBibliographicModel::mimeData(const QModelIndexList & indexes) const
    {
        if (!indexes.isEmpty()) {
            QMimeData * mimeData = new BibliographicMimeData(indexes);
            mimeData->setData(_INTERNAL_MIMETYPE_BIBLIOGRAPHICITEMS, QByteArray());
            return mimeData;
        } else {
            return 0;
        }
    }

    QStringList AbstractBibliographicModel::mimeTypes() const
    {
        return QStringList() << QLatin1String(_INTERNAL_MIMETYPE_BIBLIOGRAPHICITEMS);
    }

} // namespace Athenaeum

