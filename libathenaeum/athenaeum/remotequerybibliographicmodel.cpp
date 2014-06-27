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

#include <athenaeum/remotequerybibliographicmodel.h>
#include <athenaeum/remotequerybibliographicmodel_p.h>
#include <athenaeum/persistentbibliographicmodel.h>
#include <athenaeum/bibliographicitem_p.h>
#include <athenaeum/cJSON.h>

#include <QMetaProperty>
#include <QDebug>

namespace Athenaeum
{

    RemoteQueryBibliographicModelPrivate::RemoteQueryBibliographicModelPrivate(RemoteQueryBibliographicModel * m, const QDir & path, const QString & remoteQueryExtensionName)
        : QObject(m), m(m), state(AbstractBibliographicModel::IdleState), cache(new PersistentBibliographicModel(path, this))
    {
        // Automatically link signals
        static const char * exceptions[] = { "destroyed", "stateChanged", 0 };
        const QMetaObject * mo = cache->metaObject();
        for (int i = 0; i < mo->methodCount(); ++i) {
            QMetaMethod mt1(mo->method(i));
            // Exceptions
            const char ** except = exceptions;
            while (*except) {
                if (QString::fromAscii(mt1.signature()).startsWith(*except)) {
                    break;
                }
                ++except;
            }
            if (mt1.methodType() == QMetaMethod::Signal && !*except) {
                QMetaMethod mt2(m->metaObject()->method(m->metaObject()->indexOfMethod(mt1.signature())));
                if (mt2.methodType() == QMetaMethod::Signal) {
                    connect(cache, mt1, m, mt2);
                }
            }
        }

        // Relay signals
        connect(this, SIGNAL(stateChanged(Athenaeum::AbstractBibliographicCollection::State)),
                m, SIGNAL(stateChanged(Athenaeum::AbstractBibliographicCollection::State)));

        // Make appropriate remote query
        if (cache->property("plugin").toString().isEmpty()) {
            remoteQuery = Utopia::instantiateExtension< RemoteQuery >(remoteQueryExtensionName.toStdString());
            cache->setProperty("plugin", remoteQueryExtensionName);
        } else {
            remoteQuery = Utopia::instantiateExtension< RemoteQuery >(cache->property("plugin").toString().toStdString());
        }

        // Connect remote query
        if (remoteQuery) {
            remoteQuery.data()->setParent(this);
            connect(remoteQuery.data(), SIGNAL(fetched(Athenaeum::RemoteQueryResultSet)), this, SLOT(fetched(Athenaeum::RemoteQueryResultSet)));

            // Copy properties over to query object
            foreach (const QByteArray & key, cache->dynamicPropertyNames()) {
                remoteQuery.data()->setPersistentProperty(QString::fromUtf8(key), cache->property(key));
            }

            // Defaults for cache metadata
            if (!remoteQuery.data()->persistentProperty("limit").isValid()) setLimit(100);
            if (!remoteQuery.data()->persistentProperty("offset").isValid()) setOffset(0);
            if (!remoteQuery.data()->persistentProperty("expected").isValid()) setExpected(-1);
        }
    }

    RemoteQueryBibliographicModelPrivate::~RemoteQueryBibliographicModelPrivate()
    {
        //qDebug() << "+++++++" << offset() << limit() << expected();
        if (remoteQuery) {
            // Copy properties back over to cache from query object
            foreach (const QString & key, remoteQuery.data()->persistentPropertyNames()) {
                cache->setProperty(key.toUtf8(), remoteQuery.data()->persistentProperty(key));
            }
        }
    }

    int RemoteQueryBibliographicModelPrivate::expected() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("expected").toInt() : -1;
    }

    void RemoteQueryBibliographicModelPrivate::fetched(RemoteQueryResultSet results)
    {
        // If this a successful result? FIXME
        setOffset(results.offset + results.limit);
        setLimit(results.limit);
        setExpected(results.count);

        setState(AbstractBibliographicCollection::IdleState);

        foreach (const QVariant & variant, results.results) {
            // Add to cache
            QVariantMap map(variant.toMap());
            BibliographicItem * item = BibliographicItem::fromMap(map);
            AbstractBibliographicCollection::ItemFlags flags(item->field(AbstractBibliographicModel::ItemFlagsRole).value< AbstractBibliographicCollection::ItemFlags >());
            if (flags & AbstractBibliographicCollection::UnreadFlag && cache->rowCount() > 0) {
                cache->insertItem(cache->itemAt(0), item);
            } else {
                cache->appendItem(item);
            }
            //qDebug() << "+++" << map;
        }
    }

    int RemoteQueryBibliographicModelPrivate::limit() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("limit").toInt() : 0;
    }

    int RemoteQueryBibliographicModelPrivate::offset() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("offset").toInt() : 0;
    }

    void RemoteQueryBibliographicModelPrivate::setExpected(int expected)
    {
		if (remoteQuery) {
            remoteQuery.data()->setPersistentProperty("expected", expected);
        }
		//qDebug() << "++++++e" << offset() << limit() << this->expected();
    }

    void RemoteQueryBibliographicModelPrivate::setLimit(int limit)
    {
		if (remoteQuery) {
		    remoteQuery.data()->setPersistentProperty("limit", limit);
        }
		//qDebug() << "++++++l" << offset() << this->limit() << expected();
    }

    void RemoteQueryBibliographicModelPrivate::setOffset(int offset)
    {
		if (remoteQuery) {
            remoteQuery.data()->setPersistentProperty("offset", offset);
        }
		//qDebug() << "++++++o" << this->offset() << limit() << expected();
    }

    void RemoteQueryBibliographicModelPrivate::setState(AbstractBibliographicModel::State newState)
    {
        state = newState;
        emit stateChanged(state);
    }




    RemoteQueryBibliographicModel::RemoteQueryBibliographicModel(const QDir & path, QObject * parent)
        : AbstractBibliographicModel(parent), d(new RemoteQueryBibliographicModelPrivate(this, path))
    {
        // If empty, try to fetch
        if (d->cache->rowCount() == 0 && canFetchMore(QModelIndex())) {
            fetchMore(QModelIndex());
        }
    }

    RemoteQueryBibliographicModel::RemoteQueryBibliographicModel(const QString & remoteQueryExtensionName, const QDir & path, QObject * parent)
        : AbstractBibliographicModel(parent), d(new RemoteQueryBibliographicModelPrivate(this, path, remoteQueryExtensionName))
    {
        // If empty, try to fetch
        if (d->cache->rowCount() == 0 && canFetchMore(QModelIndex())) {
            fetchMore(QModelIndex());
        }
    }

    RemoteQueryBibliographicModel::~RemoteQueryBibliographicModel()
    {
        delete d;
    }

    bool RemoteQueryBibliographicModel::canFetchMore(const QModelIndex & parent) const
    {
        // Only top-level things exist
        if (parent.isValid() || d->state != IdleState) return false;

        return d->expected() == -1 || d->offset() + d->limit() < d->expected();
    }

    void RemoteQueryBibliographicModel::clear()
    {
        // FIXME
    }

    int RemoteQueryBibliographicModel::columnCount(const QModelIndex & index) const
    {
        return d->cache->columnCount(index);
    }

    int RemoteQueryBibliographicModel::count(ItemFlags flags) const
    {
        return d->cache->count(flags);
    }

    QVariant RemoteQueryBibliographicModel::data(const QModelIndex & index, int role) const
    {
        return d->cache->data(index, role);
    }

    void RemoteQueryBibliographicModel::fetchMore(const QModelIndex & parent)
    {
        if (d->state == IdleState && !parent.isValid()) {
            //qDebug() << "fetchMore";
            if (d->remoteQuery) {
                QVariantMap query(d->remoteQuery.data()->persistentProperty("query").toMap());
                if (!query.isEmpty()) {
                    d->setState(BusyState);
                    if (!d->remoteQuery.data()->fetch(query, d->offset(), d->limit())) {
                        d->setState(IdleState);
                    }
                }
            }
        }
    }

    Qt::ItemFlags RemoteQueryBibliographicModel::flags(const QModelIndex & index) const
    {
        return d->cache->flags(index) & (~Qt::ItemIsDropEnabled);
    }

    QVariant RemoteQueryBibliographicModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        return d->cache->headerData(section, orientation, role);
    }

    QModelIndex RemoteQueryBibliographicModel::index(int row, int column, const QModelIndex & parent) const
    {
        return d->cache->index(row, column, parent);
    }

    bool RemoteQueryBibliographicModel::insertRows(int row, int count, const QModelIndex & parent)
    {
        return d->cache->insertRows(row, count, parent);
    }

    bool RemoteQueryBibliographicModel::isReadOnly() const
    {
        return true;
    }

    bool RemoteQueryBibliographicModel::isPersistent() const
    {
        return true;
    }

    QModelIndex RemoteQueryBibliographicModel::parent(const QModelIndex & index) const
    {
        return d->cache->parent(index);
    }

    QDir RemoteQueryBibliographicModel::path() const
    {
        return d->cache->path();
    }

    qreal RemoteQueryBibliographicModel::progress() const
    {
        return -1.0;
    }

    void RemoteQueryBibliographicModel::purge()
    {
        d->cache->purge();
    }

    bool RemoteQueryBibliographicModel::removeRows(int row, int count, const QModelIndex & parent)
    {
        return removeRows(row, count, parent);
    }

    int RemoteQueryBibliographicModel::rowCount(const QModelIndex & index) const
    {
        return d->cache->rowCount(index);
    }

    bool RemoteQueryBibliographicModel::setData(const QModelIndex & index, const QVariant & value, int role)
    {
        // Data setting is done internally for persistent data roles
        if (true || (role >= AbstractBibliographicCollection::PersistentRoleCount &&
            role < AbstractBibliographicCollection::RoleCount)) {
            return d->cache->setData(index, value, role);
        } else {
            return false;
        }
    }

    void RemoteQueryBibliographicModel::setTitle(const QString & title)
    {
        d->cache->setTitle(title);
    }

    void RemoteQueryBibliographicModel::setQuery(const QString & term)
    {
        QVariantMap query;
        query["term"] = term;
        setQuery(query);
    }

    void RemoteQueryBibliographicModel::setQuery(const QVariantMap & query)
    {
        if (d->remoteQuery) {
            d->remoteQuery.data()->setPersistentProperty("query", query);

            // If empty, try to fetch
            if (d->cache->rowCount() == 0 && canFetchMore(QModelIndex())) {
                fetchMore(QModelIndex());
            }
        }
    }

    AbstractBibliographicModel::State RemoteQueryBibliographicModel::state() const
    {
        // By default persistent models are idle, though they can be corrupt
        return d->state;
    }

    QString RemoteQueryBibliographicModel::title() const
    {
        return d->cache->title();
    }

} // namespace Athenaeum
