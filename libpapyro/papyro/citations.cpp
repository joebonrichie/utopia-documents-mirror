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

#include <papyro/citations.h>
#include <papyro/utils.h>

#include <QDebug>
#include <QJsonDocument>
#include <QUuid>

namespace Papyro
{

    QVariantMap citationToMap(Spine::AnnotationHandle citation)
    {
        // Which keys should be treated as lists?
        static QStringList listKeys;
        if (listKeys.isEmpty()) {
            listKeys << "authors" << "keywords" << "links";
        }

        // Compile a citation map from this annotation (FIXME candidate for utils function)
        QVariantMap map;
        std::multimap< std::string, std::string > properties(citation->properties());
        std::multimap< std::string, std::string >::const_iterator iter(properties.begin());
        std::multimap< std::string, std::string >::const_iterator end(properties.end());
        for (; iter != end; ++iter) {
            if (iter->first.compare(0, 9, "property:") == 0) {
                // Convert the key and value
                QString key(qStringFromUnicode(iter->first.substr(9)));
                QString valueStr(qStringFromUnicode(iter->second));
                QVariant value(valueStr);
                // Parse JSON if present
                if (valueStr.startsWith("json:")) {
                    //bool ok = false;
                    value = QJsonDocument::fromJson(valueStr.mid(5).toUtf8()).toVariant();
                }
                // Add to list
                if (listKeys.contains(key)) {
                    QVariantList list(map.value(key).toList());
                    list << value;
                    value = list;
                }
                // Transfer data to citation map
                if (!value.isNull()) {
                    map[key] = value;
                }
            } else if (iter->first.compare(0, 11, "provenance:") == 0) {
                QString key(qStringFromUnicode(iter->first.substr(10)));
                QString valueStr(qStringFromUnicode(iter->second));
                if (!valueStr.isEmpty()) {
                    map[key] = valueStr;
                }
            }
        }

        return map;
    }

    std::string citationValueToUnicode(const QVariant & value)
    {
        switch (value.type()) {
        case QVariant::List:
        case QVariant::StringList:
        case QVariant::Map:
            // JSON
            return (QByteArray("json:") + QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact)).constData();
        default:
            // toString
            return unicodeFromQString(value.toString());
        }
    }

    Spine::AnnotationHandle mapToCitation(const QVariantMap & map)
    {
        Spine::AnnotationHandle citation(new Spine::Annotation);
        citation->setProperty("concept", "Citation");
        //citation->setProperty("provenance:whence", "resolution");

        QMapIterator< QString, QVariant> iter(map);
        while (iter.hasNext()) {
            iter.next();
            std::string key(unicodeFromQString(iter.key()));
            QVariant value(iter.value());
            if (value.type() == QVariant::List || value.type() == QVariant::StringList) {
                foreach (QVariant eachValue, value.toList()) {
                    citation->setProperty("property:" + key, citationValueToUnicode(eachValue));
                }
            } else {
                citation->setProperty("property:" + key, citationValueToUnicode(value));
            }
        }

        return citation;
    }

    QString refspec(const QString & uuid, const QString & keyspec)
    {
        return QString("@%1:%2").arg(uuid).arg(keyspec);
    }

    QVariantMap flatten(const QVariantList & citations)
    {
        // Final repositor of flattened citation
        QVariantMap flattened;
        QVariantList sources;
        QStringList refs;
        QVariantMap provenance;

        // Start by ordering the citations, most-important first
        // FIXME

        // Demultiplex the citations into individual top-level keys
        foreach (QVariant citation, citations) {
            QVariantMap citationMap = citation.toMap();
            if (!citationMap.contains("key")) {
                QString uuid = QUuid::createUuid().toString();
                citationMap["key"] = uuid.mid(1, uuid.size() - 2);
                citation = citationMap;
            }
            QString uuid = citationMap["key"].toString();
            QMapIterator< QString, QVariant > iter(citationMap);
            while (iter.hasNext()) {
                iter.next();
                QString key(iter.key());
                // Ignore provenance
                if (key == "provenance") {
                    continue;
                }
                QVariant value(iter.value());

                // Should this be merged?
                bool is_mergeable = (key == "links" ||
                                     key == "identifiers");
                if (is_mergeable) {
                    if (key == "links") {
                        QVariantList existing = flattened[key].toList();
                        int i = 0;
                        foreach (QVariant item, value.toList()) {
                            if (!existing.contains(item)) {
                                existing << item;
                                refs.append(QString("%1/%2").arg(refspec(uuid, key)).arg(i));
                            }
                            ++i;
                        }
                        flattened[key] = existing;
                    } else if (key == "identifiers") {
                        QVariantMap existing = flattened[key].toMap();
                        QMapIterator< QString, QVariant > i_iter(value.toMap());
                        while (i_iter.hasNext()) {
                            i_iter.next();
                            if (!existing.contains(i_iter.key())) {
                                existing[i_iter.key()] = i_iter.value();
                                refs.append(QString("%1/%2").arg(refspec(uuid, key)).arg(i_iter.key()));
                            }
                        }
                        flattened[key] = existing;
                    }
                } else {
                    if (!flattened.contains(key)) {
                        flattened[key] = value;
                        refs.append(refspec(uuid, key));
                    }
                }
            }
            sources.append(citationMap);
        }

        if (sources.size() > 0) {
            provenance["sources"] = sources;
        }
        if (refs.size() > 0) {
            provenance["refs"] = refs;
        }
        if (provenance.size() > 0) {
            flattened["provenance"] = provenance;
        }
        return flattened;
    }

}
