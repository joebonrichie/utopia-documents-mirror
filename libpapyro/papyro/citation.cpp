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

#include <papyro/abstractbibliography.h>
#include <papyro/citation_p.h>
#include <papyro/citation.h>
#include <papyro/cJSON.h>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUuid>
#include <QVariant>

#include <QDebug>

static const char * field_names[] = {
    "key",
    "title",
    "subtitle",
    "authors",
    "url",
    "volume",
    "issue",
    "year",
    "page-from",
    "page-to",
    "abstract",
    "publication-title",
    "publisher",
    "date-imported",
    "date-modified",
    "date-resolved",
    "date-published",
    "keywords",
    "type",
    "identifiers",
    "links",
    "uri",
    "originating-uri",
    "object-path",
    "flags",
    "unstructured", // This is that last field that can be persisted
    "state",
    "known",
    "userdef",
    0
};

namespace Athenaeum
{

    CitationPrivate::CitationPrivate(bool dirty)
        : fields(AbstractBibliography::MutableRoleCount - Qt::UserRole), dirty(dirty)
    {}




    /////////////////////////////////////////////////////////////////////////////////////
    // citation ////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    Citation::Citation(bool dirty)
        : d(new CitationPrivate(dirty))
    {
        QString uuid = QUuid::createUuid().toString();
        setField(AbstractBibliography::KeyRole, uuid.mid(1, uuid.size() - 2));
    }

    const QVariant & Citation::field(int role) const
    {
        static const QVariant null;
        int idx = role - Qt::UserRole;
        return (idx >= 0 && idx < d->fields.size()) ? d->fields.at(idx) : null;
    }

    CitationHandle Citation::fromJson(cJSON * object)
    {
        // Parse from JSON
        CitationHandle item(new Citation);
        if (object) {
            for (int role = Qt::UserRole; role < AbstractBibliography::PersistentRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                if (cJSON * field = cJSON_GetObjectItem(object, field_name)) {
                    switch (role) {

                    //// QDateTime
                    case AbstractBibliography::DateImportedRole:
                    case AbstractBibliography::DateModifiedRole:
                    case AbstractBibliography::DateResolvedRole:
                    case AbstractBibliography::DatePublishedRole:
                        item->setField(role, QDateTime::fromString(QString::fromUtf8(field->valuestring), Qt::ISODate));
                        break;

                    //// QStringList
                    case AbstractBibliography::AuthorsRole:
                    case AbstractBibliography::KeywordsRole: {
                        QStringList list;
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * item = cJSON_GetArrayItem(field, i)) {
                                list << QString::fromUtf8(item->valuestring);
                            }
                        }
                        if (!list.isEmpty()) {
                            item->setField(role, list);
                        }
                        break;
                    }

                    //// QVariantList or QVariantMaps
                    case AbstractBibliography::LinksRole: {
                        QVariantList list;
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * link_item = cJSON_GetArrayItem(field, i)) {
                                QVariantMap link;
                                int prop_count = cJSON_GetArraySize(link_item);
                                for (int j = 0; j < prop_count; ++j) {
                                    if (cJSON * item = cJSON_GetArrayItem(link_item, j)) {
                                        link[QString::fromUtf8(item->string)] = QString::fromUtf8(item->valuestring);
                                    }
                                }
                                list << link;
                            }
                        }
                        if (!list.isEmpty()) {
                            item->setField(role, list);
                        }
                        break;
                    }

                    //// ItemFlags
                    case AbstractBibliography::ItemFlagsRole: {
                        static QMap< QString, AbstractBibliography::ItemFlag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = AbstractBibliography::UnreadItemFlag;
                            mapping["starred"] = AbstractBibliography::StarredItemFlag;
                        }
                        AbstractBibliography::ItemFlags flags(AbstractBibliography::NoItemFlags);
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * flagName = cJSON_GetArrayItem(field, i)) {
                                flags |= mapping.value(QString::fromUtf8(flagName->valuestring), AbstractBibliography::NoItemFlags);
                            }
                        }
                        if (flags) {
                            item->setField(role, QVariant::fromValue(flags));
                        }
                        break;
                    }

                    //// QVariantMap
                    case AbstractBibliography::IdentifiersRole: {
                        QVariantMap map;
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * item = cJSON_GetArrayItem(field, i)) {
                                map[QString::fromUtf8(item->string)] = QString::fromUtf8(item->valuestring);
                            }
                        }
                        if (!map.isEmpty()) {
                            item->setField(role, map);
                        }
                        break;
                    }

                    //// QUrl
                    case AbstractBibliography::UrlRole:
                    case AbstractBibliography::ObjectFileRole:
                        item->setField(role, QUrl::fromEncoded(field->valuestring));
                        break;

                    //// QString compatible
                    default:
                        item->setField(role, QString::fromUtf8(field->valuestring));
                        break;
                    }
                }
            }
        }
        return item;
    }


    CitationHandle Citation::fromMap(const QVariantMap & variant)
    {
        // Parse from JSON
        CitationHandle citation(new Citation);
        citation->updateFromMap(variant);
        return citation;
    }

    void Citation::updateFromMap(const QVariantMap & variant)
    {
        if (!variant.isEmpty()) {
            for (int role = Qt::UserRole; role < AbstractBibliography::MutableRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }
                QVariant field(variant.value(field_name));
                if (field.isValid()) {
                    switch (role) {
                    case AbstractBibliography::ItemFlagsRole: {
                        static QMap< QString, AbstractBibliography::ItemFlag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = AbstractBibliography::UnreadItemFlag;
                            mapping["starred"] = AbstractBibliography::StarredItemFlag;
                        }
                        AbstractBibliography::ItemFlags flags(AbstractBibliography::NoItemFlags);
                        foreach (const QString & flagName, field.toStringList()) {
                            flags |= mapping.value(flagName, AbstractBibliography::NoItemFlags);
                        }
                        if (flags) {
                            setField(role, QVariant::fromValue(flags));
                        }
                        break;
                    }
                    default:
                        setField(role, field);
                        break;
                    }
                }
            }
        }
    }

    bool Citation::isBusy() const
    {
        AbstractBibliography::ItemState state = field(AbstractBibliography::ItemStateRole).value< AbstractBibliography::ItemState >();
        return state == AbstractBibliography::BusyItemState;
    }

    bool Citation::isDirty() const
    {
        return d->dirty;
    }

    bool Citation::isKnown() const
    {
        return field(AbstractBibliography::KnownRole).toBool();
    }

    bool Citation::isStarred() const
    {
        AbstractBibliography::ItemFlags flags = field(AbstractBibliography::ItemFlagsRole).value< AbstractBibliography::ItemFlags >();
        return flags & AbstractBibliography::StarredItemFlag;
    }

    void Citation::setClean()
    {
        d->dirty = false;
    }

    void Citation::setDirty()
    {
        d->dirty = true;
    }

    void Citation::setField(int role, const QVariant & data)
    {
        int idx = (role - Qt::UserRole);
        if (idx >= 0 && idx < d->fields.size()) {
            QVariant oldValue(d->fields[idx]);
            if (oldValue != data) {
                d->fields[idx] = data;
                d->dirty = true;
                emit changed(role, oldValue);
                emit changed();
            }
        }
    }

    cJSON * Citation::toJson() const
    {
        // Serialize as JSON
        cJSON * object = cJSON_CreateObject();
        for (int role = Qt::UserRole; role < AbstractBibliography::PersistentRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                switch (role) {

                //// QDateTime
                case AbstractBibliography::DateImportedRole:
                case AbstractBibliography::DateModifiedRole:
                case AbstractBibliography::DateResolvedRole:
                case AbstractBibliography::DatePublishedRole:
                    cJSON_AddStringToObject(object, field_name, field(role).toDateTime().toString(Qt::ISODate).toUtf8());
                    break;

                //// QStringList
                case AbstractBibliography::AuthorsRole:
                case AbstractBibliography::KeywordsRole: {
                    cJSON * array = cJSON_CreateArray();
                    foreach (const QString & item, field(role).toStringList()) {
                        cJSON_AddItemToArray(array, cJSON_CreateString(item.toUtf8()));
                    }
                    if (cJSON_GetArraySize(array) > 0) {
                        cJSON_AddItemToObject(object, field_name, array);
                    } else {
                        cJSON_Delete(array);
                    }
                    break;
                }

                //// QVariantList or QVariantMaps
                case AbstractBibliography::LinksRole: {
                    cJSON * array = cJSON_CreateArray();
                    foreach (const QVariant & item, field(role).toList()) {
                        cJSON * dict = cJSON_CreateObject();
                        QMapIterator< QString, QVariant > iter(item.toMap());
                        while (iter.hasNext()) {
                            iter.next();
                            cJSON_AddStringToObject(dict, iter.key().toUtf8(), iter.value().toString().toUtf8());
                        }
                        cJSON_AddItemToArray(array, dict);
                    }
                    if (cJSON_GetArraySize(array) > 0) {
                        cJSON_AddItemToObject(object, field_name, array);
                    } else {
                        cJSON_Delete(array);
                    }
                    break;
                }

                //// ItemFlags
                case AbstractBibliography::ItemFlagsRole: {
                    static QMap< AbstractBibliography::ItemFlag, const char * > mapping;
                    if (mapping.isEmpty()) {
                        mapping[AbstractBibliography::UnreadItemFlag] = "unread";
                        mapping[AbstractBibliography::StarredItemFlag] = "starred";
                    }
                    cJSON * array = cJSON_CreateArray();
                    AbstractBibliography::ItemFlags flags = field(role).value< AbstractBibliography::ItemFlags >();
                    QMapIterator< AbstractBibliography::ItemFlag, const char * > iter(mapping);
                    while (iter.hasNext()) {
                        iter.next();
                        if (flags & iter.key()) {
                            cJSON_AddItemToArray(array, cJSON_CreateString(iter.value()));
                        }
                    }
                    if (cJSON_GetArraySize(array) > 0) {
                        cJSON_AddItemToObject(object, field_name, array);
                    } else {
                        cJSON_Delete(array);
                    }
                    break;
                }

                //// QMap
                case AbstractBibliography::IdentifiersRole: {
                    cJSON * dict = cJSON_CreateObject();
                    QMapIterator< QString, QVariant > identifiers(field(role).toMap());
                    while (identifiers.hasNext()) {
                        identifiers.next();
                        cJSON_AddStringToObject(dict, identifiers.key().toUtf8(), identifiers.value().toString().toUtf8());
                    }
                    if (cJSON_GetArraySize(dict) > 0) {
                        cJSON_AddItemToObject(object, field_name, dict);
                    } else {
                        cJSON_Delete(dict);
                    }
                    break;
                }

                //// QUrl
                case AbstractBibliography::UrlRole:
                case AbstractBibliography::ObjectFileRole: {
                    QByteArray str = field(role).toUrl().toEncoded();
                    if (!str.isEmpty()) {
                        cJSON_AddStringToObject(object, field_name, str);
                    }
                    break;
                }

                //// QString compatible
                default: {
                    QString str = field(role).toString();
                    if (!str.isEmpty()) {
                        cJSON_AddStringToObject(object, field_name, str.toUtf8());
                    }
                    break;
                }
                }
            }
        }
        if (cJSON_GetArraySize(object) == 0) {
            cJSON_Delete(object);
            object = 0;
        }
        return object;
    }

    QVariantMap Citation::toMap() const
    {
        QVariantMap map;
        for (int role = Qt::UserRole; role < AbstractBibliography::MutableRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                switch (role) {
                case AbstractBibliography::ItemFlagsRole: {
                    QStringList flagNames;
                    static QMap< AbstractBibliography::ItemFlag, QString > mapping;
                    if (mapping.isEmpty()) {
                        mapping[AbstractBibliography::UnreadItemFlag] = "unread";
                        mapping[AbstractBibliography::StarredItemFlag] = "starred";
                    }
                    AbstractBibliography::ItemFlags flags = field(role).value< AbstractBibliography::ItemFlags >();
                    QMapIterator< AbstractBibliography::ItemFlag, QString > iter(mapping);
                    while (iter.hasNext()) {
                        iter.next();
                        if (flags & iter.key()) {
                            flagNames << iter.value();
                        }
                    }
                    if (!flagNames.isEmpty()) {
                        map[field_name] = flagNames;
                    }
                    break;
                }
                default:
                    map[field_name] = field(role);
                    break;
                }
            }
        }
        return map;
    }

    bool Citation::operator == (const Citation & other) const
    {
        return d == other.d;
    }

    bool Citation::operator != (const Citation & other) const
    {
        return !(*this == other);
    }

} // namespace Athenaeum
