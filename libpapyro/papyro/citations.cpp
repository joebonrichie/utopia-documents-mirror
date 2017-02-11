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
        citation->setProperty("provenance:whence", "resolution");

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
}
