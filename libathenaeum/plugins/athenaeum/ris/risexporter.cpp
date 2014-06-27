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

#include "risexporter.h"

#include <athenaeum/abstractbibliographiccollection.h>

#include <QList>
#include <QString>

#include <QDebug>

using namespace Athenaeum;

bool RISExporter::doExport(const QModelIndexList & indexList, const QString & filename)
{
    QString separator = "  - ";
    QString entryGap = "\n\n";
    QStringList risData;

    // Type
    QMap <QString, QString > types;
    types["conference abstract"] = "ABST";
    types["book"] = "BOOK";
    types["book chapter"] = "CHAP";
    types["conference paper"] = "CONF";
    types["theses"] = "THES";
    types["article"] = "JOUR";
    types["report"] = "RPRT";
    types["newspaper article"] = "NEWS";

    // Mapping
    typedef QPair< QString, AbstractBibliographicCollection::Roles > Mapping;
    QVector< Mapping > translation;
    translation << Mapping("T1", AbstractBibliographicCollection::TitleRole);
    translation << Mapping("ST", AbstractBibliographicCollection::SubtitleRole);
    translation << Mapping("AB", AbstractBibliographicCollection::AbstractRole);
    translation << Mapping("LI", AbstractBibliographicCollection::UrlRole);
    translation << Mapping("UR", AbstractBibliographicCollection::DocumentUriRole);
    translation << Mapping("VL", AbstractBibliographicCollection::VolumeRole);
    translation << Mapping("IS", AbstractBibliographicCollection::IssueRole);
    translation << Mapping("PY", AbstractBibliographicCollection::YearRole);
    translation << Mapping("PG", AbstractBibliographicCollection::PageFromRole);
    translation << Mapping("EP", AbstractBibliographicCollection::PageToRole);
    translation << Mapping("T2", AbstractBibliographicCollection::PublicationTitleRole);
    translation << Mapping("SP", AbstractBibliographicCollection::PublisherRole);

    foreach (const QModelIndex & index, indexList) {
        QString type(index.data(AbstractBibliographicCollection::TypeRole).toString());
        if (types.contains(type)) {
            risData.append("TY" + separator + types[type]);
        } else {
            risData.append("TY" + separator + types["article"]);
        }

        // Translate standard fields
        foreach (const Mapping & pair, translation) {
            QString data(index.data(pair.second).toString());
            if (!data.isEmpty()) {
                risData.append(pair.first + separator + data);
            }
        }

        // Authors
        foreach (const QString & author, index.data(AbstractBibliographicCollection::AuthorsRole).toStringList()) {
            risData.append("AU" + separator + author);
        }

        // Keywords
        foreach (const QString & kw, index.data(AbstractBibliographicCollection::KeywordsRole).toStringList()) {
            risData.append("KW" + separator + kw);
        }

        // Identifiers
        QMap< QString, QString > identifiers;
        identifiers["doi"] = "DO";
        // FIXME More RIS codes for identifiers!
        QMapIterator< QString, QVariant > id_iter(index.data(AbstractBibliographicCollection::IdentifiersRole).toMap());
        while (id_iter.hasNext()) {
            id_iter.next();
            if (identifiers.contains(id_iter.key())) {
                risData.append(identifiers[id_iter.key()] + separator + id_iter.value().toString());
            }
        }
        risData.append("ER" + separator);
        risData.append(entryGap);
    }

    // Write out
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(risData.join("\n").toUtf8());
        file.close();
        return true;
    } else {
        return false;
    }
}

QStringList RISExporter::extensions() const
{
    QStringList exts;
    exts << "ris";
    return exts;
}

QString RISExporter::name() const
{
    return "RIS";
}

bool RISExporter::multipleFiles() const
{
    return false;
}
