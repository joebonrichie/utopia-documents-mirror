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

#include <athenaeum/bibliographicitem_p.h>
#include <athenaeum/cJSON.h>

#include <QString>
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
    "date-modified",
    "keywords",
    "type",
    "identifiers",
    "uri",
    "pdf",
    "flags"
};

namespace Athenaeum
{

    /////////////////////////////////////////////////////////////////////////////////////
    // BibliographicItem ////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    BibliographicItem::BibliographicItem(bool dirty)
        : fields(AbstractBibliographicModel::RoleCount - Qt::UserRole), dirty(dirty)
    {
        QString key = QUuid::createUuid();
        setField(AbstractBibliographicModel::KeyRole, key.mid(1, key.size() - 2));
    }

    // Construct a copy (includng key) of another item
    BibliographicItem::BibliographicItem(BibliographicItem * item)
        : fields(item->fields), dirty(true)
    {}

    const QVariant & BibliographicItem::field(int index) const
    {
        return fields.at(index - Qt::UserRole);
    }

    BibliographicItem * BibliographicItem::fromJson(cJSON * object)
    {
        // Parse from JSON
        BibliographicItem * item = 0;
        if (object) {
            item = new BibliographicItem(false);
            for (int role = Qt::UserRole; role < AbstractBibliographicCollection::PersistentRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (cJSON * field = cJSON_GetObjectItem(object, field_name)) {
                    switch (role) {

                    //// QDateTime
                    case AbstractBibliographicModel::DateModifiedRole:
                        item->setField(role, QDateTime::fromString(QString::fromUtf8(field->valuestring), Qt::ISODate));
                        break;

                    //// QStringList
                    case AbstractBibliographicModel::AuthorsRole:
                    case AbstractBibliographicModel::KeywordsRole: {
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

                    //// ItemFlags
                    case AbstractBibliographicModel::ItemFlagsRole: {
                        static QMap< QString, AbstractBibliographicCollection::ItemFlag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = AbstractBibliographicCollection::UnreadFlag;
                            mapping["important"] = AbstractBibliographicCollection::ImportantFlag;
                        }
                        AbstractBibliographicCollection::ItemFlags flags(AbstractBibliographicCollection::NoFlags);
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * flagName = cJSON_GetArrayItem(field, i)) {
                                flags |= mapping.value(QString::fromUtf8(flagName->valuestring), AbstractBibliographicCollection::NoFlags);
                            }
                        }
                        if (flags) {
                            item->setField(role, QVariant::fromValue(flags));
                        }
                        break;
                    }

                    //// QVariantMap
                    case AbstractBibliographicModel::IdentifiersRole: {
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
                    case AbstractBibliographicModel::UrlRole:
                    case AbstractBibliographicModel::PdfRole:
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


    BibliographicItem * BibliographicItem::fromMap(const QVariantMap & variant)
    {
        // Parse from JSON
        BibliographicItem * item = 0;
        if (!variant.isEmpty()) {
            item = new BibliographicItem(false);
            for (int role = Qt::UserRole; role < AbstractBibliographicCollection::PersistentRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                QVariant field(variant.value(field_name));
                if (field.isValid()) {
                    switch (role) {
                    case AbstractBibliographicModel::ItemFlagsRole: {
                        static QMap< QString, AbstractBibliographicCollection::ItemFlag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = AbstractBibliographicCollection::UnreadFlag;
                            mapping["important"] = AbstractBibliographicCollection::ImportantFlag;
                        }
                        AbstractBibliographicCollection::ItemFlags flags(AbstractBibliographicCollection::NoFlags);
                        foreach (const QString & flagName, field.toStringList()) {
                            flags |= mapping.value(flagName, AbstractBibliographicCollection::NoFlags);
                        }
                        if (flags) {
                            item->setField(role, QVariant::fromValue(flags));
                        }
                        break;
                    }
                    default:
                        item->setField(role, field);
                        break;
                    }
                }
            }
        }
        return item;
    }

    bool BibliographicItem::isDirty() const
    {
        return dirty;
    }

    void BibliographicItem::setClean()
    {
        dirty = false;
    }

    void BibliographicItem::setDirty()
    {
        dirty = true;
    }

    void BibliographicItem::setField(int index, const QVariant & data)
    {
        fields[index - Qt::UserRole] = data;
        dirty = true;
    }

    cJSON * BibliographicItem::toJson() const
    {
        // Serialize as JSON
        cJSON * object = cJSON_CreateObject();
        for (int role = Qt::UserRole; role < AbstractBibliographicCollection::PersistentRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                switch (role) {

                //// QDateTime
                case AbstractBibliographicModel::DateModifiedRole:
                    cJSON_AddStringToObject(object, field_name, field(role).toDateTime().toString(Qt::ISODate).toUtf8());
                    break;

                //// QStringList
                case AbstractBibliographicModel::AuthorsRole:
                case AbstractBibliographicModel::KeywordsRole: {
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

                //// ItemFlags
                case AbstractBibliographicModel::ItemFlagsRole: {
                    static QMap< AbstractBibliographicCollection::ItemFlag, const char * > mapping;
                    if (mapping.isEmpty()) {
                        mapping[AbstractBibliographicCollection::UnreadFlag] = "unread";
                        mapping[AbstractBibliographicCollection::ImportantFlag] = "important";
                    }
                    cJSON * array = cJSON_CreateArray();
                    AbstractBibliographicCollection::ItemFlags flags = field(role).value< AbstractBibliographicCollection::ItemFlags >();
                    QMapIterator< AbstractBibliographicCollection::ItemFlag, const char * > iter(mapping);
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
                case AbstractBibliographicModel::IdentifiersRole: {
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
                case AbstractBibliographicModel::UrlRole:
                case AbstractBibliographicModel::PdfRole: {
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

    QVariantMap BibliographicItem::toMap() const
    {
        QVariantMap map;
        for (int role = Qt::UserRole; role < AbstractBibliographicCollection::PersistentRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                switch (role) {
                case AbstractBibliographicModel::ItemFlagsRole: {
                    QStringList flagNames;
                    static QMap< AbstractBibliographicCollection::ItemFlag, QString > mapping;
                    if (mapping.isEmpty()) {
                        mapping[AbstractBibliographicCollection::UnreadFlag] = "unread";
                        mapping[AbstractBibliographicCollection::ImportantFlag] = "important";
                    }
                    AbstractBibliographicCollection::ItemFlags flags = field(role).value< AbstractBibliographicCollection::ItemFlags >();
                    QMapIterator< AbstractBibliographicCollection::ItemFlag, QString > iter(mapping);
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

} // namespace Athenaeum
