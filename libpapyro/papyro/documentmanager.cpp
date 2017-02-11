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

#include <papyro/documentmanager.h>
#include <papyro/documentmanager_p.h>

#include <papyro/documentfactory.h>

#include <boost/bind.hpp>

#include <QDomDocument>
#include <QFile>
#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent>
#include <QStringList>
#include <QTime>
#include <QUrl>
#include <Qt>

#include <QDebug>

namespace Papyro
{

    namespace
    {

        QString ampersand_encode(const QString &string) {
            QString encoded;
            for(int i=0;i<string.size();++i) {
                QChar ch = string.at(i);
                if(ch.unicode() > 255)
                    encoded += QString("&#%1;").arg((int)ch.unicode());
                else
                    encoded += ch;
            }
            return encoded;
        }

    }




    DocumentManagerPrivate::DocumentManagerPrivate(DocumentManager * manager)
        : QObject(manager), manager(manager), serviceManager(Kend::ServiceManager::instance())
    {
        // Gather document factories
        foreach (DocumentFactory * factory, Utopia::instantiateAllExtensions< DocumentFactory >()) {
            factories.append(factory);
        }

        // Populate model from service manager
        for (int i = 0; i < serviceManager->count(); ++i) {
            Kend::Service * service = serviceManager->serviceAt(i);
            onServiceAdded(service);
        }

        // Connect all signals from manager
        connect(serviceManager.get(), SIGNAL(serviceAdded(Kend::Service*)), this, SLOT(onServiceAdded(Kend::Service*)));
        connect(serviceManager.get(), SIGNAL(serviceRemoved(Kend::Service*)), this, SLOT(onServiceRemoved(Kend::Service*)));
    }

    DocumentManagerPrivate::~DocumentManagerPrivate()
    {
        foreach (DocumentFactory * factory, factories) {
            delete factory;
        }
    }

    void DocumentManagerPrivate::onResolveFinished()
    {
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        QEventLoop * loop = qobject_cast< QEventLoop * >(reply->request().originatingObject());

        QVariant redirectsVariant = reply->property("__redirects");
        int redirects = redirectsVariant.isNull() ? 20 : redirectsVariant.toInt();

        // Redirect?
        QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectedUrl.isValid()) {
            if (redirectedUrl.isRelative()) {
                QString redirectedAuthority = redirectedUrl.authority();
                redirectedUrl = reply->url().resolved(redirectedUrl);
                if (!redirectedAuthority.isEmpty()) {
                    redirectedUrl.setAuthority(redirectedAuthority);
                }
            }
            if (redirects > 0) {
                QNetworkRequest request = reply->request();
                request.setUrl(redirectedUrl);
                request.setOriginatingObject(loop);
                QNetworkReply * reply = networkAccessManager()->get(request);
                reply->setProperty("__redirects", redirects - 1);
                connect(reply, SIGNAL(finished()), this, SLOT(onResolveFinished()));
                return;
            } else {
                // Too many times FIXME
            }
        } else {
            switch (reply->error()) {
            case QNetworkReply::NoError: {
                // Parse out the reply
                QDomDocument doc;
                doc.setContent(reply);
                QDomElement documentRefElem = doc.documentElement();
                if (documentRefElem.tagName() == "documentref" && documentRefElem.attribute("version") == "0.3") {
                    QString documentUri(documentRefElem.attribute("id"));
                    if (!documentUri.isEmpty()) {
                        reply->setProperty("documentUri", documentUri);
                    }
                }
                break;
            }
            default:
                // FIXME error
                break;
            }
        }

        // Quit event loop and finish
        loop->quit();
    }

    void DocumentManagerPrivate::onServiceAdded(Kend::Service * service)
    {
        services.append(service);
        // Resolve all documents FIXME
    }

    void DocumentManagerPrivate::onServiceRemoved(Kend::Service * service)
    {
        services.removeAll(service);
        // Remove all documents FIXME
    }

    void DocumentManagerPrivate::onServiceStateChanged(Kend::Service * service, Kend::Service::ServiceState state)
    {
        // Re-resolve a document
    }

    void DocumentManagerPrivate::registerDocument(Kend::Service * service, Spine::DocumentHandle document)
    {
        // Start with resolving an ID for this document with
        QString documentUri = resolveDocument(service, document);
    }

    QString DocumentManagerPrivate::resolveDocument(Kend::Service * service, Spine::DocumentHandle document)
    {
        QEventLoop loop;

        // Set off resolution in the background
        static QString documentref = QString("                                      \n\
            <?xml version=\"1.0\" encoding=\"UTF-8\" ?>                             \n\
            <documentref xmlns=\"http://utopia.cs.manchester.ac.uk/kend\"           \n\
                         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"    \n\
                         xsi:type=\"UnidentifiedDocumentReference\"                 \n\
                         version=\"0.7\">                                           \n\
                %1                                                                  \n\
            </documentref>                                                          \n\
        ").trimmed();
        QStringList evidence;
        foreach (std::string fingerprint, document->fingerprints()) {
            evidence << QString("<evidence type=\"fingerprint\" srctype=\"document\">%1</evidence>").arg(ampersand_encode(QString::fromStdString(fingerprint)));
        }

        QUrl url(service->resourceUrl(Kend::Service::DocumentsResource));
        QByteArray requestData = documentref.arg(evidence.join("")).toUtf8();
        qDebug() << documentref.arg(evidence.join(""));
        static QString requestType("application/x-kend+xml;version=0.7;type=documentref;documentref");
        QNetworkRequest request(url);
        request.setOriginatingObject(&loop);
        QNetworkReply * reply = service->post(request, requestData, requestType);
        connect(reply, SIGNAL(finished()), this, SLOT(onResolveFinished()));

        // Loop until complete (onResolveFinished will quit this loop)
        loop.exec();

        // Get the document URI back from the reply object
        QString documentUri = reply->property("documentUri").toString();
        reply->deleteLater();

        return documentUri;
    }

    void DocumentManagerPrivate::unregisterDocument(Kend::Service * service, Spine::DocumentHandle document)
    {
    }




    DocumentManager::DocumentManager(QObject * parent)
        : QObject(parent), d(new DocumentManagerPrivate(this))
    {}

    boost::shared_ptr< DocumentManager > DocumentManager::instance()
    {
        static boost::weak_ptr< DocumentManager > singleton;
        boost::shared_ptr< DocumentManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< DocumentManager >(new DocumentManager());
            singleton = shared;
        }
        return shared;
    }

    Spine::DocumentHandle DocumentManager::open(QIODevice * io)
    {
        QTime t; t.start();
        Spine::DocumentHandle document;
        // Only valid, open, and readable IODevices can be read from.
        if (io) {
            if (io->isOpen() || io->open(QIODevice::ReadOnly)) {
                if (io->isReadable()) {
                    // Find a factory that can load this document
                    foreach (DocumentFactory * factory, d->factories) {
                        QEventLoop eventLoop;
                        QFutureWatcher< Spine::DocumentHandle > watcher;
                        connect(&watcher, SIGNAL(finished()), &eventLoop, SLOT(quit()));
                        QFuture< Spine::DocumentHandle > future = QtConcurrent::run(boost::bind(static_cast< Spine::DocumentHandle (DocumentFactory::*)( const QByteArray & ) >(&DocumentFactory::create), factory, io->readAll()));
                        watcher.setFuture(future);
                        eventLoop.exec();
                        if ((document = future.result())) {
                            //registerDocument(document);
                            break;
                        }
                    }
                }
            }
        }
        return document;
    }

    Spine::DocumentHandle DocumentManager::open(const QString & filename)
    {
        QFile file(filename);
        return open(&file);
    }

    void DocumentManager::registerDocument(Spine::DocumentHandle document)
    {
        if (document) {
            QString binaryHash(QString::fromStdString(document->filehash()));
            if (!d->registry.contains(binaryHash)) {
                foreach (Kend::Service * service, d->services) {
                    if (!d->registry[binaryHash].contains(service)) {
                        QString documentUri(d->resolveDocument(service, document));
                        if (!documentUri.isEmpty()) {
                            d->registry[binaryHash][service] = qMakePair(Spine::WeakDocumentHandle(document), documentUri);
                        }
                    }
                }
            }
        }
    }

    void DocumentManager::unregisterDocument(Spine::DocumentHandle document)
    {
    }

}
