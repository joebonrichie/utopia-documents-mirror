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
    "volume",
    "issue",
    "year",
    "page-from",
    "page-to",
    "abstract",
    "publication-title",
    "publisher",
    "keywords",
    "type",
    "identifiers",
    "links",
    "uri",
    "originating-uri",
    "object-path",
    "unstructured",
    "provenance",
    "state",
    "flags",
    "date-imported",
    "date-resolved",
    "url",
    "date-published",
    "date-modified", // This is that last field that can be persisted
    "known",
    "userdef",
    0
};

namespace Athenaeum
{

    CitationPrivate::CitationPrivate(bool dirty)
        : fields(Citation::MutableRoleCount - Qt::UserRole), dirty(dirty)
    {}




    /////////////////////////////////////////////////////////////////////////////////////
    // citation ////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    Citation::Citation(bool dirty)
        : d(new CitationPrivate(dirty))
    {
        QString uuid = QUuid::createUuid().toString();
        setField(KeyRole, uuid.mid(1, uuid.size() - 2));
    }

    const QVariant & Citation::field(int role) const
    {
        static const QVariant null;
        int idx = role - Qt::UserRole;
        return (idx >= 0 && idx < d->fields.size()) ? d->fields.at(idx) : null;
    }

    static QVariant jsonToQVariant(cJSON * obj)
    {
        if (obj) {
            switch (obj->type) {
            case cJSON_False:
                return false;
            case cJSON_True:
                return true;
            case cJSON_Number:
                return obj->valuedouble;
            case cJSON_String:
                return QString::fromUtf8(obj->valuestring);
            case cJSON_Array: {
                QVariantList array;
                int count = cJSON_GetArraySize(obj);
                for (int i = 0; i < count; ++i) {
                    array << jsonToQVariant(cJSON_GetArrayItem(obj, i));
                }
                return array;
            }
            case cJSON_Object: {
                QVariantMap object;
                int count = cJSON_GetArraySize(obj);
                for (int i = 0; i < count; ++i) {
                    cJSON * child = cJSON_GetArrayItem(obj, i);
                    object[QString::fromUtf8(child->string)] = jsonToQVariant(child);
                }
                return object;
            }
            case cJSON_NULL:
            default:
                break;
            }
        }
        return QVariant();
    }

    static cJSON * qVariantToJson(const QVariant & variant)
    {
        cJSON * obj = 0;
        switch (variant.type()) {
        case QMetaType::Bool:
            obj = variant.toBool() ? cJSON_CreateTrue() : cJSON_CreateFalse();
            break;
        case QMetaType::Double:
            obj = cJSON_CreateNumber(variant.toDouble());
            break;
        case QMetaType::QString:
            obj = cJSON_CreateString(variant.toString().toUtf8().constData());
            break;
        case QMetaType::QVariantList:
        case QMetaType::QStringList: {
            obj = cJSON_CreateArray();
            foreach (QVariant item, variant.toList()) {
                cJSON_AddItemToArray(obj, qVariantToJson(item));
            }
            break;
        }
        case QMetaType::QVariantMap: {
            obj = cJSON_CreateObject();
            QVariantMap map(variant.toMap());
            QMapIterator< QString, QVariant > iter(map);
            while (iter.hasNext()) {
                iter.next();
                cJSON_AddItemToObject(obj, iter.key().toUtf8().constData(), qVariantToJson(iter.value()));
            }
            break;
        }
        default:
            obj = cJSON_CreateNull();
            break;
        }
        return obj;
    }

    CitationHandle Citation::fromJson(cJSON * object)
    {
        // Parse from JSON
        CitationHandle item(new Citation);
        if (object) {
            for (int role = Qt::UserRole; role < PersistentRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                if (cJSON * field = cJSON_GetObjectItem(object, field_name)) {
                    switch (role) {

                    //// QDateTime
                    case DateImportedRole:
                    case DateModifiedRole:
                    case DateResolvedRole:
                    case DatePublishedRole:
                        item->setField(role, QDateTime::fromString(QString::fromUtf8(field->valuestring), Qt::ISODate));
                        break;

                    //// Flags
                    case FlagsRole: {
                        static QMap< QString, Flag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = UnreadFlag;
                            mapping["starred"] = StarredFlag;
                        }
                        Citation::Flags flags(NoFlags);
                        int count = cJSON_GetArraySize(field);
                        for (int i = 0; i < count; ++i) {
                            if (cJSON * flagName = cJSON_GetArrayItem(field, i)) {
                                flags |= mapping.value(QString::fromUtf8(flagName->valuestring), NoFlags);
                            }
                        }
                        if (flags) {
                            item->setField(role, QVariant::fromValue(flags));
                        }
                        break;
                    }

                    //// QUrl
                    case UrlRole:
                    case ObjectFileRole:
                        item->setField(role, QUrl::fromEncoded(field->valuestring));
                        break;

                    //// Standard QVariant compatible
                    case AuthorsRole:
                    case KeywordsRole:
                    case LinksRole:
                    case IdentifiersRole:
                    case ProvenanceRole:
                    default:
                        item->setField(role, jsonToQVariant(field));
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
            for (int role = Qt::UserRole; role < MutableRoleCount; ++role) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }
                QVariant field(variant.value(field_name));
                if (field.isValid()) {
                    switch (role) {
                    case FlagsRole: {
                        static QMap< QString, Flag > mapping;
                        if (mapping.isEmpty()) {
                            mapping["unread"] = UnreadFlag;
                            mapping["starred"] = StarredFlag;
                        }
                        Citation::Flags flags(NoFlags);
                        foreach (const QString & flagName, field.toStringList()) {
                            flags |= mapping.value(flagName, NoFlags);
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
        State state = field(StateRole).value< State >();
        return state == BusyState;
    }

    bool Citation::isDirty() const
    {
        return d->dirty;
    }

    bool Citation::isKnown() const
    {
        return field(KnownRole).toBool();
    }

    bool Citation::isStarred() const
    {
        Citation::Flags flags = field(FlagsRole).value< Citation::Flags >();
        return flags & StarredFlag;
    }

    QString Citation::roleTitle(Role role)
    {
        switch (role) {
        case Citation::KeyRole: return QString("Key");
        case Citation::TitleRole: return QString("Title");
        case Citation::SubTitleRole: return QString("Subtitle");
        case Citation::AuthorsRole: return QString("Authors");
        case Citation::UrlRole: return QString("Url");
        case Citation::VolumeRole: return QString("Volume");
        case Citation::IssueRole: return QString("Issue");
        case Citation::YearRole: return QString("Year");
        case Citation::PageFromRole: return QString("Start Page");
        case Citation::PageToRole: return QString("End Page");
        case Citation::AbstractRole: return QString("Abstract");
        case Citation::PublicationTitleRole: return QString("Publication Title");
        case Citation::PublisherRole: return QString("Publisher");
        case Citation::DateImportedRole: return QString("Date Imported");
        case Citation::DateModifiedRole: return QString("Date Modified");
        case Citation::DateResolvedRole: return QString("Date Resolved");
        case Citation::DatePublishedRole: return QString("Date Published");
        case Citation::KeywordsRole: return QString("Keywords");
        case Citation::TypeRole: return QString("Type");
        case Citation::IdentifiersRole: return QString("Identifiers");
        case Citation::DocumentUriRole: return QString("Document URI");
        case Citation::OriginatingUriRole: return QString("Imported Path");
        case Citation::ObjectFileRole: return QString("Filename");
        case Citation::FlagsRole: return QString("Flags");
        case Citation::ProvenanceRole: return QString("Provenance");
        case Citation::UnstructuredRole: return QString("Unstructured");
        case Citation::UserDefRole: return QString("UserDef");
        default: return QString();
        }
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
        for (int role = Qt::UserRole; role < PersistentRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                switch (role) {

                //// QDateTime
                case DateImportedRole:
                case DateModifiedRole:
                case DateResolvedRole:
                case DatePublishedRole:
                    cJSON_AddStringToObject(object, field_name, field(role).toDateTime().toString(Qt::ISODate).toUtf8());
                    break;

                //// QStringList
                case AuthorsRole:
                case KeywordsRole: {
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
                case LinksRole: {
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

                //// Flags
                case FlagsRole: {
                    static QMap< Flag, const char * > mapping;
                    if (mapping.isEmpty()) {
                        mapping[UnreadFlag] = "unread";
                        mapping[StarredFlag] = "starred";
                    }
                    cJSON * array = cJSON_CreateArray();
                    Citation::Flags flags = field(role).value< Citation::Flags >();
                    QMapIterator< Flag, const char * > iter(mapping);
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
                case ProvenanceRole:
                case IdentifiersRole: {
                    cJSON * dict = qVariantToJson(field(role));
                    if (cJSON_GetArraySize(dict) > 0) {
                        cJSON_AddItemToObject(object, field_name, dict);
                    } else {
                        cJSON_Delete(dict);
                    }
                    break;
                }

                //// QUrl
                case UrlRole:
                case ObjectFileRole: {
                    QByteArray str = field(role).toUrl().toEncoded();
                    if (!str.isEmpty()) {
                        cJSON_AddStringToObject(object, field_name, str);
                    }
                    break;
                }

                //// QString compatible
                default:
                    cJSON_AddItemToObject(object, field_name, qVariantToJson(field(role)));
                    break;
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
        for (int role = Qt::UserRole; role < MutableRoleCount; ++role) {
            if (field(role).isValid()) {
                const char * field_name = field_names[role-Qt::UserRole];
                if (!field_name) {
                    break;
                }

                switch (role) {
                case FlagsRole: {
                    QStringList flagNames;
                    static QMap< Flag, QString > mapping;
                    if (mapping.isEmpty()) {
                        mapping[UnreadFlag] = "unread";
                        mapping[StarredFlag] = "starred";
                    }
                    Citation::Flags flags = field(role).value< Citation::Flags >();
                    QMapIterator< Flag, QString > iter(mapping);
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
