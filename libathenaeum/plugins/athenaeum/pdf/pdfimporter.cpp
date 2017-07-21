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

#include "pdfimporter.h"
#include <papyro/resolver.h>
#include <crackle/PDFDocument.h>

#include <QEventLoop>
#include <QFile>
#include <QIODevice>
#include <boost/scoped_ptr.hpp>

#include <QDebug>

PDFImporter::PDFImporter()
{
    foreach (Athenaeum::Resolver * resolver, Utopia::instantiateAllExtensions< Athenaeum::Resolver >()) {
        resolvers.insert(resolver);
    }
}

PDFImporter::~PDFImporter()
{
    foreach (Athenaeum::Resolver * resolver, resolvers) {
        delete resolver;
    }
}

// Import the file only and add to the index given
bool PDFImporter::import(Athenaeum::LibraryModel * model, QModelIndex & index, QIODevice * io)
{
    if (!index.isValid())
    {
        return this->import(model, io);
    }

    QString fileName(model->generateObjectPath(index));

    QFile * inputFile = (QFile *) io;


    if (!inputFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "import(fileonly) Failed to open input file";
        return false;
    }

    QFile objectFile(fileName);
    qDebug() << objectFile.open(QIODevice::WriteOnly);
    objectFile.write(inputFile->readAll());
    objectFile.close();
    inputFile->close();

    qDebug() << "setting file name " << fileName;

    model->setFileName(index, fileName);

    return true;
}

bool PDFImporter::import(Athenaeum::LibraryModel * model, QIODevice * io)
{
    // Create a new record in the DB and save the PDF into its object file location
    int count(model->rowCount());
    model->insertRow(count);
    QModelIndex index(model->index(count, 0));
    QString fileName(model->generateObjectPath(index));
    Spine::DocumentHandle document(new Crackle::PDFDocument());
    QFile * inputFile = (QFile *) io;
    if (!inputFile->open(QIODevice::ReadOnly))
    {
        qDebug() << "import() Failed to open input file";
        return false;
    }
    QFile objectFile(fileName);
    objectFile.open(QIODevice::WriteOnly);
    objectFile.write(inputFile->readAll());
    objectFile.close();
    inputFile->close();
    document->readFile(fileName.toUtf8());
    qDebug() << fileName.toUtf8();
    if (document->isOK()) {
        model->setFileName(index, fileName);

        // Fill in metadata if possible
        foreach (Athenaeum::Resolver * resolver, resolvers) {
            QVariantMap resolved(resolver->resolve(document, QVariantList()));

            // Identifiers
            QMap< QString, QString > identifiers;
            if (resolved.contains("doi")) {
                identifiers.insert("doi", resolved["doi"].toString());
            }
            if (resolved.contains("pmid")) {
                identifiers.insert("pmid", resolved["pmid"].toString());
            }
            model->setIdentifiers(index, identifiers);

            // Authors
            if (resolved.contains("authors")) {
                QList< QPair< QStringList, QStringList > > authors;
                QVariant v(resolved["authors"]);
                QList< QVariant > list = v.toList();
                for (int authorIndex = 0; authorIndex < list.count(); ++authorIndex)
                {
                    QList < QVariant > author = list.at(authorIndex).toList();
                    if (author.count() == 2) {
                        QStringList givenNameList = author[0].toString().split(" ");
                        QStringList familyNameList = author[1].toString().split(" ");
                        authors.append(qMakePair(givenNameList, familyNameList));
                    }
                }
                model->setAuthors(index, authors);
            }

            model->setTitle(index, resolved.value("title", "").toString());
            model->setStartPage(index, resolved.value("startpage", "").toString());
            model->setEndPage(index, resolved.value("endpage", "").toString());
            model->setSubTitle(index, resolved.value("subtitle", "").toString());
            model->setUrl(index, resolved.value("url", "").toString());
            model->setVolume(index, resolved.value("volume", "").toString());
            model->setIssue(index, resolved.value("issue", "").toString());
            model->setYear(index, resolved.value("year", "").toString());
            model->setPublicationTitle(index, resolved.value("publicationtitle", "").toString());
            model->setPublisher(index, resolved.value("publisher", "").toString());
            model->setKeywords(index, QStringList(resolved.value("keywords", "").toString()));
            model->setType(index, resolved.value("type", "").toString());


            //TODO All other types
            /*
            KeyRole = Qt::UserRole,
            TitleRole,
            SubTitleRole,
            AuthorsRole,
            UrlRole,
            VolumeRole,
            IssueRole,
            YearRole,
            PageFromRole,
            PageToRole,
            AbstractRole,
            PublicationTitleRole,
            PublisherRole,
            DateModifiedRole,
            KeywordsRole,
            TypeRole,
            IdentifiersRole,
            DocumentUriRole,
            */
        }

        return true;
    }

    // Clean up in case of failure
    objectFile.remove();
    model->removeRow(count);

    return false;
}

QStringList PDFImporter::extensions() const
{
    QStringList exts;
    exts << "pdf";
    return exts;
}

QString PDFImporter::name() const
{
    return "PDF";
}

bool PDFImporter::supports(QIODevice * io) const
{
    return false;
}
