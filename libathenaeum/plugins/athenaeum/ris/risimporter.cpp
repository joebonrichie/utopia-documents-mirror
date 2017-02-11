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

#include "risimporter.h"

#include <athenaeum/bibliographicitem.h>

#include <QMap>
#include <QString>
#include <QDebug>

namespace {

class RISParser {

public:

    RISParser(QIODevice *io) : _io(io)
    {}

    bool advanceToNextTag()
    {
        while(!isTag(_next_line) && !_io->atEnd()) {
            _next_line=QString::fromUtf8(_io->readLine());
        }

        return isTag(_next_line);
    }

    bool readNextEntry(QString *tag, QString *data)
    {
        if(advanceToNextTag()) {
            *tag=_next_line.left(2);
            *data=_next_line.mid(6).trimmed();
            _next_line.clear();

            while(!_io->atEnd() && !isTag(_next_line)) {
                _next_line=QString::fromUtf8(_io->readLine());

                if(!isTag(_next_line)) {
                    *data+=" " + _next_line.trimmed();
                    _next_line.clear();
                }
            }

            return true;
        }
        return false;
    }

    bool readNextRecord(QString *tag, QString *data)
    {
        while ( readNextEntry(tag, data) ) {
            if(*tag=="TY") {
                return true;
            }
        }
        return false;
    }

    bool isTag(QString line)
    {
        return (line.length() >= 6 &&
                line[0] >= 'A' && line[0] <= 'Z' &&
                ((line[1] >= 'A' && line[1]<='Z') || (line[1] >= '0' && line[1] <= '9')) &&
                line[2] == ' ' &&
                line[3] == ' ' &&
                line[4] == '-' &&
                line[5] == ' ');
    }

private:
    QIODevice * _io;
    QString _next_line;
};

}

bool RISImporter::import(Athenaeum::AbstractBibliography * model, QIODevice * io)
{
    // Parse the QIODevice and populate the model
    RISParser parser(io);

    QString tag;
    QString entry;
    qDebug() << "Importing RIS data";

    while(parser.readNextRecord(&tag, &entry)) {

        QStringList authors;
        QStringList keywords;
        QVariantMap identifiers;

        QString type;
        QString start_page;
        QString end_page;
        QString title;
        QString subtitle;
        QString url;
        QString abstract;
        QString publication_title;
        QString publisher;
        QString year;
        QString issue;
        QString volume;

        if(entry.startsWith("ABST", Qt::CaseInsensitive)) {
            type="conference abstract";
        } else if(entry.startsWith("BOOK", Qt::CaseInsensitive)) {
            type="book";
        } else if(entry.startsWith("CHAP", Qt::CaseInsensitive)) {
            type="book chapter";
        } else if(entry.startsWith("CONF", Qt::CaseInsensitive)) {
            type="conference paper";
        } else if(entry.startsWith("THES", Qt::CaseInsensitive)) {
            type="theses";
        } else if(entry.startsWith("JOUR", Qt::CaseInsensitive)) {
            type="article";
        } else if(entry.startsWith("RPRT", Qt::CaseInsensitive)) {
            type="report";
        } else if(entry.startsWith("NEWS", Qt::CaseInsensitive)) {
            type="newspaper article";
        } else {
            qDebug() << "Skipping unhandled RIS record: " << entry;
        }

        if(!type.isEmpty()) {
            while(parser.readNextEntry(&tag, &entry) && tag!="ER") {

                if(tag=="AU" || tag == "A2" || tag == "A1") {
                    if(tag=="A1") {
                        authors.push_front(entry);
                    } else {
                        authors.append(entry);
                    }
                }
                else if(tag=="DO") {
                    // non standard DOI entry - formatting is "relaxed" at best
//                    QRegExp doi("(?:DOI|doi|[Dd]igital\\s+[Oo]bject\\s+[Ii][Dd](?:entifier))\\s*:?\\s*(10\\.\\d+/[^%\"\\#\\x20\\t\\r\\n]+)");
                    QRegExp doi("(10\\.\\d+/[^%\"\\#\\s]+)");
                    int pos = doi.indexIn(entry);
                    if (pos > -1) {
                        identifiers["doi"]=doi.cap(1);
                    } else {
                        qDebug() << "Failed to match DOI in " << entry;
                    }
                }
                else if(tag=="SP") {
                    // often lots of junk in page details
                    QRegExp pages("^(\\w+)[\\s\\x2013\\x2014\\-]*(\\w*)");

                    int pos = pages.indexIn(entry);
                    if (pos > -1) {
                        start_page = pages.cap(1);
                        if(end_page.isEmpty()) {
                            end_page = pages.cap(2);
                        }

                    } else {
                        qDebug() << "Could not parse start page number(s): " << entry;
                    }
                }
                else if(tag=="EP") {
                    QRegExp pages("^(\\w+)");
                    int pos = pages.indexIn(entry);
                    if (pos > -1) {
                        end_page = pages.cap(1);
                    } else {
                        qDebug() << "Could not parse end page number: " << entry;
                    }
                }
                else if(tag=="T1" || tag =="TI") {
                    if(tag=="T1" || title.isEmpty()) {
                        title=entry;
                    }
                }
// T2 seem to be used for journal/conference not subtitle
//                else if(tag=="T2") {
//                    subtitle=entry;
//                }
                else if(tag=="L1" || tag=="UR") {
                    if(tag=="UR" || url.isEmpty()) {
                        url=entry;
                    }
                }
                else if(tag=="AB") {
                    abstract=entry;
                }
                else if(tag=="JF" || tag=="JO" || tag=="JA" || tag=="J1" || tag=="J2" || tag=="T2") {
                    // prefer full name
                    if(tag=="JF" || publication_title.isEmpty()) {
                        publication_title=entry;
                    }
                }
                else if(tag=="PB") {
                    publisher=entry;
                }
                else if(tag=="PY" || tag=="Y1") {
                    QRegExp pages("(\\d+)(.*)");
                    int pos = pages.indexIn(entry);
                    if (pos > -1) {
                        year = pages.cap(1);
                    } else {
                        qDebug() << "Failed to parse year: " << entry;
                    }
                }
                else if(tag=="IS") {
                    issue=entry;
                }
                else if(tag=="VL") {
                    volume=entry;
                }
                else if(tag=="KW") {
                    keywords << entry;
                }
                else {
                    qDebug() << "Skipping unhandled field " << tag;
                }

            }

            qDebug() << "type" << type;
            qDebug() << "authors" << authors;
            qDebug() << "identifiers" << identifiers;
            qDebug() << "start_page" << start_page;
            qDebug() << "end_page" << end_page;
            qDebug() << "title" << title;
            //qDebug() << "subtitle" << subtitle;
            qDebug() << "url" << url;
            qDebug() << "abstract" << abstract;
            qDebug() << "publicationTitle" << publication_title;
            qDebug() << "publisher" << publisher;
            qDebug() << "year" << year;
            qDebug() << "issue" << issue;
            qDebug() << "volume" << volume;
            qDebug() << "keywords" << keywords;

            if(model) {
                Athenaeum::BibliographicItemHandle item(new Athenaeum::BibliographicItem);

                item->setField(Athenaeum::AbstractBibliography::TypeRole, type);
                item->setField(Athenaeum::AbstractBibliography::AuthorsRole, authors);
                item->setField(Athenaeum::AbstractBibliography::IdentifiersRole, identifiers);
                item->setField(Athenaeum::AbstractBibliography::PageFromRole, start_page);
                item->setField(Athenaeum::AbstractBibliography::PageToRole, end_page);
                item->setField(Athenaeum::AbstractBibliography::TitleRole, title);
                item->setField(Athenaeum::AbstractBibliography::UrlRole, url);
                item->setField(Athenaeum::AbstractBibliography::AbstractRole, abstract);
                item->setField(Athenaeum::AbstractBibliography::PublicationTitleRole, publication_title);
                item->setField(Athenaeum::AbstractBibliography::PublisherRole, publisher);
                item->setField(Athenaeum::AbstractBibliography::YearRole, year);
                item->setField(Athenaeum::AbstractBibliography::IssueRole, issue);
                item->setField(Athenaeum::AbstractBibliography::VolumeRole, volume);
                item->setField(Athenaeum::AbstractBibliography::KeywordsRole, keywords);

                model->appendItem(item);
            }

        }
    }

    return true;
}

QStringList RISImporter::extensions() const
{
    QStringList exts;
    exts << "ris"; // Guessed at this
    return exts;
}

QString RISImporter::name() const
{
    return "RIS";
}

bool RISImporter::supports(QIODevice * io) const
{
    // If we find a record (skipping crap)
    RISParser parser(io);
    QString tag, entry;
    return parser.readNextRecord(&tag, &entry);
}

