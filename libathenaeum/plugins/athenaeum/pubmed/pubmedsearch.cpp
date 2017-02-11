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

#include "pubmedsearch.h"

#include <QByteArray>
#include <QDebug>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPair>
#include <QString>
#include <QStringList>

#include <QDebug>

namespace Athenaeum
{
    PubmedSearchPrivate::PubmedSearchPrivate(QObject * parent)
    : QObject(parent)
    {
        this->parentObject = 0;
        this->working = false;
        this->cancelled = false;
        this->network = new QNetworkAccessManager;
        QObject::connect(this->network, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));

        this->baseFetchURL = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi";
        this->baseSearchURL = "http://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi";
    }

    bool PubmedSearchPrivate::busy()
    {
        return this->working;
    }

    void PubmedSearchPrivate::cancel()
    {
        this->cancelled = true;
    }

    void PubmedSearchPrivate::fetchPMIDS(QString queryKey, QString webEnv)
    {
        qDebug() << "queryKey = " << queryKey;
        qDebug() << "webEnv = " << webEnv;

        QPair<QByteArray, QByteArray> dbParam = qMakePair(QByteArray("db"), QByteArray("pubmed"));
        //	QPair<QByteArray, QByteArray> idParam = qMakePair(QByteArray("id"), QByteArray(ids.toUtf8()));
        QPair<QByteArray, QByteArray> retModeParam = qMakePair(QByteArray("retmode"), QByteArray("xml"));
        QPair<QByteArray, QByteArray> webenvParam = qMakePair(QByteArray("WebEnv"), QByteArray(webEnv.toUtf8()));
        QPair<QByteArray, QByteArray> querykeyParam = qMakePair(QByteArray("query_key"), QByteArray(queryKey.toUtf8()));
        QPair<QByteArray, QByteArray> retTypeParam = qMakePair(QByteArray("rettype"), QByteArray("abstract"));
        QPair<QByteArray, QByteArray> toolParam = qMakePair(QByteArray("tool"), QByteArray("utopialibrary"));

        QList< QPair<QByteArray, QByteArray> > paramList;

        paramList << dbParam << retModeParam << querykeyParam << webenvParam << retTypeParam << toolParam;

        QUrl url(this->baseFetchURL);
        url.setEncodedQueryItems(paramList);

        qDebug() << "URL IS " << url;

        QNetworkRequest request(url);
        this->network->get(request);
    }

    int PubmedSearchPrivate::parseEFetchResponse(QString response)
    {
        QDomDocument efetch("EFetch Response");
        efetch.setContent(response);

        //qDebug() << response;

        QDomNode eFetchResultNode = efetch.firstChildElement("PubmedArticleSet");

        if (!eFetchResultNode.isNull())
        {
            QDomElement articleSetElement = eFetchResultNode.toElement();

            QDomNodeList articleNodeList = articleSetElement.elementsByTagName("PubmedArticle");

            qDebug() << "There are " << articleNodeList.count() << " articles";

            for (int articleIndex = 0 ; articleIndex < articleNodeList.count() ; ++articleIndex)
            {
                QDomNode articleNode = articleNodeList.at(articleIndex);
                QDomElement articleElement = articleNode.toElement();

                QDomNode medlineCitationNode = articleNode.firstChildElement("MedlineCitation");

                if (!medlineCitationNode.isNull())
                {
                    QDomNode articleNode = medlineCitationNode.firstChildElement("Article");

                    if (!articleNode.isNull())
                    {
                        QString title = articleNode.firstChildElement("ArticleTitle").toElement().text();
                        QString abstract = articleNode.firstChildElement("Abstract").toElement().text();

                        QDomNode journalNode = articleNode.firstChildElement("Journal");

                        QString publicationTitle = journalNode.firstChildElement("Title").toElement().text();
                        QString publicationISOTitle = journalNode.firstChildElement("ISOAbbreviation").toElement().text();

                        QDomNode journalIssueNode = journalNode.firstChildElement("JournalIssue");


                        QString volume = journalIssueNode.firstChildElement("Volume").toElement().text();
                        QString issue = journalIssueNode.firstChildElement("Issue").toElement().text();

                        QDomNode journalPubDateNode = journalIssueNode.firstChildElement("PubDate");
                        QString year = journalPubDateNode.firstChildElement("Year").toElement().text();
                        QString month = journalPubDateNode.firstChildElement("Month").toElement().text();
                        QString day = journalPubDateNode.firstChildElement("Day").toElement().text();

                        QDomNode authorListNode = articleNode.firstChildElement("AuthorList");

                        QDomNodeList authorList = articleNode.toElement().elementsByTagName("Author");


                        this->model->insertRow(this->model->rowCount());
                        QModelIndex index(this->model->index(this->model->rowCount() - 1, 0));

                        this->model->setTitle(index, title);
                        this->model->setPublicationTitle(index, publicationTitle);
                        this->model->setVolume(index, volume);
                        this->model->setYear(index, year);
                        this->model->setAbstract(index, abstract);



                        QList< QPair< QStringList, QStringList > > authors;

                        for (int authorIndex = 0 ; authorIndex < authorList.count(); ++ authorIndex)
                        {
                            QDomNode authorNode = authorList.at(authorIndex);
                            QString foreName = authorNode.firstChildElement("ForeName").toElement().text();
                            QString lastName = authorNode.firstChildElement("LastName").toElement().text();

                            //qDebug() << "Author : " << foreName << " " << lastName;

                            QStringList authorFamily = lastName.split(" ");
                            QStringList authorGiven = foreName.split(" ");

                            authors.append(qMakePair(authorGiven, authorFamily));
                        }

                        this->model->setAuthors(index, authors);
                        QString key = this->model->key(index);
                        if (key.isEmpty())
                        {
                            // Do something else here?
                            qDebug() << "Ignoring record without resolvable key";
                            this->model->removeRow(index.row());
                        }
                        else
                        {
                            // FIXME : Scan for duplicates

                            this->model->setKey(index, key);
                        }

                        //qDebug() << abstract;
                    }
                    else
                    {
                        qDebug() << "disaster";
                        exit(1);
                    }

                }

                QDomNode pubmedNode = articleNode.firstChildElement("PubmedData");

                if (!pubmedNode.isNull())
                {
                    QDomNode articleIDNode = pubmedNode.firstChildElement("ArticleIdList");

                    QDomNodeList idList = articleIDNode.toElement().elementsByTagName("ArticleId");
                    for (int idIndex = 0 ; idIndex < idList.count() ; ++idIndex)
                    {
                        QDomElement idElement = idList.at(idIndex).toElement();
                        QString id = idElement.text();
                        //qDebug() << idElement.attribute("IdType") << " : " << id;

                    }
                }
                else
                {
                    qDebug() << "No PubmedData";
                }

                //qDebug() << "\n\n";
            }
            return articleNodeList.count();
        }
        else
        {
            qDebug() << "Got dodgy results (2)";
        }

        return -1;
    }

    int PubmedSearchPrivate::parseESearchResponse(QString response)
    {

        //qDebug() << response;
        //qDebug() << "\n\n\n";
        QDomDocument esearch("ESearch Response");
        esearch.setContent(response);

        QDomNode eSearchResultNode = esearch.firstChildElement("eSearchResult");

        if (!eSearchResultNode.isNull())
        {
            QDomNode countNode = eSearchResultNode.firstChildElement("Count");
            QDomNode queryKeyNode = eSearchResultNode.firstChildElement("QueryKey");
            QDomNode webenvNode = eSearchResultNode.firstChildElement("WebEnv");
            //		QDomNode retMaxNode = eSearchResultNode.firstChildElement("RetMax");
            //		QDomNode retStartNode = eSearchResultNode.firstChildElement("RetStart");

            QDomElement countElement = countNode.toElement();
            QDomElement queryKeyElement = queryKeyNode.toElement();
            QDomElement webenvElement = webenvNode.toElement();
            //		QDomElement retMaxElement = retMaxNode.toElement();
            //		QDomElement retStartElement = retStartNode.toElement();

            qDebug() << "Returned " << countElement.text() << " results";

            if (countElement.text().toInt() != 0)
            {
                this->fetchPMIDS(queryKeyElement.text(), webenvElement.text());
            }
            return countElement.text().toInt();
        }
        else
        {
            qDebug() << "Got dodgy results (1)";
        }
        return -1;
    }

    void PubmedSearchPrivate::replyFinished(QNetworkReply *reply)
    {
        qDebug() << "got reply to " << reply->request().url().path();

        if (this->cancelled)
        {
            qDebug() << "Search was cancelled, stopping.";
            this->working = false;
            this->cancelled = false;
            emit terminated(model);
            return;
        }

        if (reply->error() == QNetworkReply::NoError)
        {
            qDebug() << "No error";

            if (reply->request().url().path() == "/entrez/eutils/esearch.fcgi")
            {
                QIODevice *result = reply;
                QByteArray res = result->readAll();
                QString xmlstring = res;
                int results = this->parseESearchResponse(res);
                emit searchComplete(model, results);
                // If there are no results, finish fetch as well
                if (results == 0)
                {
                    this->working = false;
                    emit fetchComplete(model, results);
                }
            }
            else if (reply->request().url().path() == "/entrez/eutils/efetch.fcgi")
            {
                this->working = false;
                QIODevice *result = reply;
                QByteArray res = result->readAll();
                QString xmlstring = res;

                int results = this->parseEFetchResponse(res);
                emit fetchComplete(model, results);
            }
            else
            {
                this->working = false;
                qFatal("Unrecognised response");
                exit(1);

            }
        }
        else
        {
            qDebug() << "Error was " << reply->error();
            this->working = false;
            emit terminated(model);
        }

    }

    void PubmedSearchPrivate::sendQuery(QList< QList < QString > > clauses, bool matchAll)
    {
        this->working = true;

        // Basic Pubmed query creation

        QString query;
        for (int i = 0; i < clauses.count(); ++i)
        {
            qDebug() << "Clause: " << clauses.at(i);

            QString clauseQuery;
            if (clauses.at(i).at(1).compare("does not contain") == 0)
            {
                clauseQuery += "NOT ";
            }
            //clauseQuery += "\"";
            clauseQuery += clauses.at(i).at(2);
            //clauseQuery += "\"";
            clauseQuery += "[";
            clauseQuery += clauses.at(i).at(0);
            clauseQuery += "]";

            query += clauseQuery;

            if (i < clauses.count() - 1)
            {
                if (matchAll)
                {
                    query += " AND ";
                }
                else
                {
                    query += " OR ";
                }
            }
        }

        QPair<QByteArray, QByteArray> dbParam = qMakePair(QByteArray("db"), QByteArray("pubmed"));
        QPair<QByteArray, QByteArray> termParam = qMakePair(QByteArray("term"), QUrl::toPercentEncoding(query));

        // don't need to actually return any PMIDs, since we use the WebEnv environment to retrieve
        // them at the eSearch level. So for now, ask for 0 results to keep this lump of XML
        // as small as possible
        QPair<QByteArray, QByteArray> retstartParam = qMakePair(QByteArray("retstart"), QByteArray("0"));
        QPair<QByteArray, QByteArray> retmaxParam = qMakePair(QByteArray("retmax"), QByteArray("0"));
        QPair<QByteArray, QByteArray> toolParam = qMakePair(QByteArray("tool"), QByteArray("utopialibrary"));
        QPair<QByteArray, QByteArray> historyParam = qMakePair(QByteArray("usehistory"), QByteArray("y"));

        QList< QPair<QByteArray, QByteArray> > paramList;

        paramList << dbParam << termParam << retstartParam << retmaxParam << toolParam << historyParam;

        QUrl url(this->baseSearchURL);
        url.setEncodedQueryItems(paramList);

        QNetworkRequest request(url);
        this->network->get(request);
    }

    void PubmedSearchPrivate::setLibraryModel(LibraryModel * libraryModel)
    {
        model = libraryModel;
    }

    void PubmedSearchPrivate::setReceiver(QObject * parent)
    {
        if (this->parentObject == 0)
        {
            this->parentObject = parent;

            connect(this, SIGNAL(fetchComplete(LibraryModel *, int)), parent, SLOT(completed(LibraryModel *, int)));
            connect(this, SIGNAL(searchComplete(LibraryModel *, int)), parent, SLOT(partCompleted(LibraryModel *, int)));
            connect(this, SIGNAL(terminated(LibraryModel *)), parent, SLOT(terminated(LibraryModel *)));
        }
    }

    /**********************************************/

    PubmedSearch::PubmedSearch()
    {
        p = new PubmedSearchPrivate;
    }

    PubmedSearch::~PubmedSearch()
    {
        delete p;
    }

    bool PubmedSearch::busy()
    {
        return p->busy();
    }

    void PubmedSearch::cancel()
    {
        p->cancel();
    }

    QString PubmedSearch::name() const
    {
        return "Pubmed";
    }

    void PubmedSearch::sendQuery(QList< QList < QString > > clauses, bool matchAll)
    {
        p->sendQuery(clauses, matchAll);
    }

    void PubmedSearch::setLibraryModel(LibraryModel * libraryModel)
    {
        this->model = libraryModel;
        p->setLibraryModel(libraryModel);
    }

    void PubmedSearch::setReceiver(QObject * parent)
    {
        p->setReceiver(parent);
    }
}

