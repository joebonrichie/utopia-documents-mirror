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

#include <papyro/cslengineadapter.h>

#include <QDate>
#include <QStringList>

namespace Papyro
{

    namespace
    {

        typedef QVariant (*conversionFn)(const QVariant & names);

        QVariant convert_date(int y, int m, int d)
        {
            QVariantMap date;
            QVariantList dateParts;
            QVariantList datePart;
            datePart << y;
            if (m > 0) {
                datePart << m;
                if (d > 0) {
                    datePart << d;
                }
            }
            dateParts.push_back(datePart);
            date["date-parts"] = dateParts;
            return date;
        }

        QVariant convert_year(const QVariant & input)
        {
            return convert_date(input.toString().toInt(), 0, 0);
        }

        QVariant convert_names(const QVariant & input)
        {
            QVariantList names;
            foreach (const QString & inputName, input.toStringList()) {
                QVariantMap name;
                name["given"] = inputName.section(",", 1).trimmed();
                name["family"] = inputName.section(",", 0, 0).trimmed();
                names.push_back(name);
            }
            return names;
        }

        QVariant convert_str(const QVariant & input)
        {
            return input.toString();
        }
    }

    QVariantMap convert_to_cslengine(const QVariantMap & metadata)
    {
        typedef QPair< QString, conversionFn > pair;
        static QMap< QString, pair > conversionMap;
        if (conversionMap.isEmpty()) {
            conversionMap["key"] = pair("id", convert_str);
            conversionMap["title"] = pair("title", convert_str);
            conversionMap["authors"] = pair("author", convert_names);
            conversionMap["url"] = pair("URL", convert_str);
            conversionMap["volume"] = pair("volume", convert_str);
            conversionMap["issue"] = pair("issue", convert_str);
            conversionMap["year"] = pair("issued", convert_year);
            conversionMap["abstract"] = pair("abstract", convert_str);
            conversionMap["publication-title"] = pair("container-title", convert_str);
            conversionMap["publication-issn"] = pair("ISSN", convert_str);
            conversionMap["publication-isbn"] = pair("ISBN", convert_str);
            conversionMap["publisher"] = pair("publisher", convert_str);
            conversionMap["pages"] = pair("page", convert_str);
            conversionMap["label"] = pair("citation-label", convert_str);
        }

        static QMap< QString, QString > identifierMap;
        if (identifierMap.isEmpty()) {
            identifierMap["pmcid"] = "PMCID";
            identifierMap["pmid"] = "PMID";
            identifierMap["doi"] = "DOI";
        }

        QVariantMap converted;

        QMapIterator< QString, pair > citer(conversionMap);
        while (citer.hasNext()) {
            citer.next();
            if (metadata.contains(citer.key())) {
                converted[citer.value().first] = citer.value().second(metadata[citer.key()]);
            }
        }

        QMapIterator< QString, QString > iiter(identifierMap);
        while (iiter.hasNext()) {
            iiter.next();
            QVariantMap identifiers(metadata.value("identifiers").toMap());
            if (metadata.contains(iiter.key())) {
                converted[iiter.value()] = metadata[iiter.key()];
            } else if (identifiers.contains(iiter.key())) {
                converted[iiter.value()] = identifiers[iiter.key()];
            }
        }

        return converted;
    }

} // namespace Papyro
