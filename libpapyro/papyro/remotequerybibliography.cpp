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

#include <papyro/remotequerybibliography.h>
#include <papyro/remotequerybibliography_p.h>
#include <papyro/citation.h>
#include <papyro/bibliography.h>
#include <papyro/cJSON.h>
#include <papyro/librarymodel.h>
#include <papyro/persistencemodel.h>

#include <QDateTime>
#include <QModelIndex>
#include <QMetaProperty>

#include <QDebug>

namespace Athenaeum
{

    RemoteQueryBibliographyPrivate::RemoteQueryBibliographyPrivate(RemoteQueryBibliography * remoteQueryBibliography,
                                                                   const QString & remoteQueryExtensionName)
        : QObject(remoteQueryBibliography),
          remoteQueryBibliography(remoteQueryBibliography),
          libraryModel(Athenaeum::LibraryModel::instance()),
          persistenceModel(new NoPersistenceModel(this))
    {
        // Make appropriate remote query
        remoteQuery = Utopia::instantiateExtension< RemoteQuery >(remoteQueryExtensionName.toStdString());
        setProperty("plugin", remoteQueryExtensionName);

        // Connect remote query
        if (remoteQuery) {
            remoteQuery.data()->setParent(this);
            connect(remoteQuery.data(), SIGNAL(fetched(Athenaeum::RemoteQueryResultSet)), this, SLOT(fetched(Athenaeum::RemoteQueryResultSet)));

            // Copy properties over to query object
            foreach (const QByteArray & key, remoteQueryBibliography->dynamicPropertyNames()) {
                remoteQuery.data()->setPersistentProperty(QString::fromUtf8(key), remoteQueryBibliography->property(key));
            }

            // Defaults for cache metadata
            if (!remoteQuery.data()->persistentProperty("limit").isValid()) setLimit(100);
            if (!remoteQuery.data()->persistentProperty("offset").isValid()) setOffset(0);
            if (!remoteQuery.data()->persistentProperty("expected").isValid()) setExpected(-1);
        }
    }

    RemoteQueryBibliographyPrivate::~RemoteQueryBibliographyPrivate()
    {}

    int RemoteQueryBibliographyPrivate::expected() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("expected").toInt() : -1;
    }

    void RemoteQueryBibliographyPrivate::fetched(RemoteQueryResultSet results)
    {
        // If this a successful result? FIXME
        setOffset(results.offset + results.limit);
        setLimit(results.limit);
        setExpected(results.count);

        remoteQueryBibliography->setState(AbstractBibliography::IdleState);

        foreach (const QVariant & variant, results.results) {
            // Add to cache
            QVariantMap map(variant.toMap());
            CitationHandle item = Citation::fromMap(map);

            // Set provenance
            QVariantMap prov = item->field(Citation::ProvenanceRole).toMap();
            QVariantMap origin;
            origin["action"] = "remotesearch";
            origin["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            {
                QVariantMap context = query();
                {
                    QVariantMap plugin;
                    plugin["name"] = property("plugin");
                    plugin["title"] = remoteQuery.data()->title();
                    context["plugin"] = plugin;
                }
                origin["context"] = context;
            }
            prov["origin"] = origin;
            item->setField(Citation::ProvenanceRole, prov);

            // See if we can match this item to an existing library entry
            QVariantMap ids(item->field(Citation::IdentifiersRole).toMap());
            QMapIterator< QString, QVariant > iter(ids);
            Athenaeum::Bibliography * master = libraryModel->master();
            while (iter.hasNext()) {
                iter.next();
                QString id(iter.key() + ":" + iter.value().toString());
                if (CitationHandle found = master->itemForId(id)) {
                    item = found;
                    break;
                }
            }
            Citation::Flags flags(item->field(Citation::FlagsRole).value< Citation::Flags >());
            if (flags & Citation::UnreadFlag && remoteQueryBibliography->rowCount() > 0) {
                remoteQueryBibliography->prependItem(item);
            } else {
                remoteQueryBibliography->appendItem(item);
            }
            //qDebug() << "+++" << map;
        }
    }

    int RemoteQueryBibliographyPrivate::limit() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("limit").toInt() : 0;
    }

    int RemoteQueryBibliographyPrivate::offset() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("offset").toInt() : 0;
    }

    QVariantMap RemoteQueryBibliographyPrivate::query() const
    {
        return remoteQuery ? remoteQuery.data()->persistentProperty("query").toMap() : QVariantMap();
    }

    void RemoteQueryBibliographyPrivate::setExpected(int expected)
    {
		if (remoteQuery) {
            remoteQuery.data()->setPersistentProperty("expected", expected);
        }
		//qDebug() << "++++++e" << offset() << limit() << this->expected();
    }

    void RemoteQueryBibliographyPrivate::setLimit(int limit)
    {
		if (remoteQuery) {
		    remoteQuery.data()->setPersistentProperty("limit", limit);
        }
		//qDebug() << "++++++l" << offset() << this->limit() << expected();
    }

    void RemoteQueryBibliographyPrivate::setOffset(int offset)
    {
		if (remoteQuery) {
            remoteQuery.data()->setPersistentProperty("offset", offset);
        }
		//qDebug() << "++++++o" << this->offset() << limit() << expected();
    }




    RemoteQueryBibliography::RemoteQueryBibliography(const QString & remoteQueryExtensionName, QObject * parent)
        : Bibliography(parent), d(new RemoteQueryBibliographyPrivate(this, remoteQueryExtensionName))
    {
        // If empty, try to fetch
        if (rowCount() == 0 && canFetchMore(QModelIndex())) {
            fetchMore(QModelIndex());
        }
    }

    RemoteQueryBibliography::~RemoteQueryBibliography()
    {
        delete d;
    }

    bool RemoteQueryBibliography::canFetchMore(const QModelIndex & parent) const
    {
        // Only top-level things exist
        if (parent.isValid() || state() != IdleState) return false;

        return d->expected() == -1 || (d->offset() + d->limit()) < d->expected();
    }

    void RemoteQueryBibliography::fetchMore(const QModelIndex & parent)
    {
        if (state() == IdleState && !parent.isValid()) {
            if (d->remoteQuery) {
                QVariantMap query(d->remoteQuery.data()->persistentProperty("query").toMap());
                if (!query.isEmpty()) {
                    setState(BusyState);
                    if (!d->remoteQuery.data()->fetch(query, d->offset(), d->limit())) {
                        setState(IdleState);
                    }
                }
            }
        }
    }

    bool RemoteQueryBibliography::isReadOnly() const
    {
        return true;
    }

    PersistenceModel * RemoteQueryBibliography::persistenceModel() const
    {
        return d->persistenceModel;
    }

    void RemoteQueryBibliography::setQuery(const QString & term)
    {
        QVariantMap query;
        query["term"] = term;
        setQuery(query);
    }

    void RemoteQueryBibliography::setQuery(const QVariantMap & query)
    {
        if (d->remoteQuery) {
            d->remoteQuery.data()->setPersistentProperty("query", query);

            // If empty, try to fetch
            if (rowCount() == 0 && canFetchMore(QModelIndex())) {
                fetchMore(QModelIndex());
            }
        }
    }

} // namespace Athenaeum
