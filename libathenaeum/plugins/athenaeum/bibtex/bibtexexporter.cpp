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

#include "bibtexexporter.h"

#include "version_p.h"
#include <papyro/abstractbibliography.h>

#include <QList>
#include <QString>

#include <QDebug>

using namespace Athenaeum;

static QString encodeValue(QString value)
{
    return value.replace("{", "\\{").replace("}", "\\}").replace("&", "\\&");
}

bool BibTeXExporter::doExport(const QModelIndexList & indexList, const QString & filename)
{
    // Type
    QMap <QString, QString > types;
    types["conference abstract"] = "CONFERENCE";
    types["book"] = "BOOK";
    types["book chapter"] = "INBOOK";
    types["conference paper"] = "INPROCEEDINGS";
    types["theses"] = "PHDTHESIS";
    types["article"] = "ARTICLE";
    types["report"] = "TECHREPORT";
    types["newspaper article"] = "MISC";

    // Mapping
    typedef QPair< QString, Citation::Role > Mapping;
    QVector< Mapping > translation;
    translation << Mapping("title", Citation::TitleRole);
    translation << Mapping("abstract", Citation::AbstractRole);
    translation << Mapping("url", Citation::UrlRole);
    translation << Mapping("volume", Citation::VolumeRole);
    translation << Mapping("number", Citation::IssueRole);
    translation << Mapping("year", Citation::YearRole);
    translation << Mapping("journal", Citation::PublicationTitleRole);
    translation << Mapping("publisher", Citation::PublisherRole);

    // Write out
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        // Write header: creating application and encoding information (utf8)
        file.write(("% This file was created with Utopia " + Utopia::versionString() + "\n").toUtf8());
        file.write("% Encoding: UTF8");

        // For each entry, serialise the BibTeX citation for it
        typedef QMap< QString, QString > QStringMap;
        QList< QStringMap > entries;
        QMap< QString, int > idCache;
        QMap< QString, int > idCount;
        foreach (const QModelIndex & index, indexList) {
            QString id;
            QString firstAuthorSurname;
            QStringMap fields;

            fields["type"] = types.value(index.data(Citation::TypeRole).toString(), "MISC");

            // Translate standard fields
            foreach (const Mapping & pair, translation) {
                QString data(index.data(pair.second).toString());
                if (!data.isEmpty()) {
                    fields[pair.first] = data;
                }
            }

            // Authors
            QStringList authors;
            foreach (const QString & author, index.data(Citation::AuthorsRole).toStringList()) {
                QString forenames = author.section(", ", 1, 1);
                QString surnames = author.section(", ", 0, 0);
                if (firstAuthorSurname.isEmpty()) {
                    firstAuthorSurname = surnames.remove(QRegExp("\\s+"));
                }
                authors << (forenames + " " + surnames).trimmed();
            }
            if (!authors.isEmpty()) {
                fields["author"] = authors.join(" and ");
            }

            // Pages
            QStringList pagesList;
            pagesList << index.data(Citation::PageFromRole).toString();
            pagesList << index.data(Citation::PageToRole).toString();
            QString pages = pagesList.join("-");
            if (!pages.isEmpty() && pages != "-") {
                fields["pages"] = pages;
            }

            // Keywords
            QStringList keywords = index.data(Citation::KeywordsRole).toStringList();
            if (!keywords.isEmpty()) {
                fields["keywords"] = keywords.join(", ");
            }

            // Identifiers
            QStringMap identifiers;
            identifiers["doi"] = "doi";
            // FIXME More BibTeX codes for identifiers!
            QMapIterator< QString, QVariant > id_iter(index.data(Citation::IdentifiersRole).toMap());
            while (id_iter.hasNext()) {
                id_iter.next();
                if (identifiers.contains(id_iter.key())) {
                    fields[identifiers[id_iter.key()]] = id_iter.value().toString();
                }
            }

            // Calculate an ID for this entry based on first author surname and year
            fields["id"] = firstAuthorSurname + fields.value("year", "");
            int idx(idCount.value(id, 0));
            idCount[id] = ++idx;

            entries << fields;
        }

        int numericalIdx = 1;
        foreach (const QStringMap & fields, entries) {
            QString id = fields["id"];
            if (id.isEmpty()) {
                id = QString("bib%1").arg(numericalIdx++);
            } else {
                int idx(idCache.value(id, -1));
                idCache[id] = ++idx;
                if (idCount[id] > 1) {
                    qDebug() << id << idx;
                    id += QString("abcdefghijklmnopqrstuvwxyz").at(idx-1);
                }
            }

            // Write entry
            file.write("\n\n@");
            file.write(fields["type"].toUtf8());
            file.write("{");
            file.write(id.toUtf8());
            QMapIterator< QString, QString > iter(fields);
            while (iter.hasNext()) {
                iter.next();
                if (iter.key() != "type" && iter.key() != "id") {
                    file.write(",\n");
                    file.write("    ");
                    file.write(iter.key().toUtf8());
                    file.write(" = {");
                    file.write(encodeValue(iter.value()).toUtf8());
                    file.write("}");
                }
            }
            file.write("\n}");
        }

        return true;
    } else {
        return false;
    }
}

QStringList BibTeXExporter::extensions() const
{
    QStringList exts;
    exts << "bib";
    return exts;
}

QString BibTeXExporter::name() const
{
    return "BibTeX";
}

bool BibTeXExporter::multipleFiles() const
{
    return false;
}

