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

#include <utopia2/networkaccessmanager.h>
#include <utopia2/networkaccessmanager_p.h>
#include <utopia2/pacproxyfactory.h>
#include <utopia2/certificateerrordialog.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <QEvent>
#include <QMap>
#include <QMutexLocker>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSignalMapper>
#include <QSslCertificate>
#include <QSslError>
#include <QThreadStorage>
#include <QTimer>

#include <QDebug>

// So that a QSslCertificate can be placed in a QSet
uint qHash(const QSslCertificate & certificate) { return qHash(certificate.digest()); }

namespace Utopia
{

    NetworkAccessManagerPrivate::NetworkAccessManagerPrivate(NetworkAccessManager * manager)
        : QObject(manager), manager(manager), paused(false)
    {
        QSettings conf;
        conf.beginGroup("Networking");
        conf.beginGroup("Security");
        int count = conf.beginReadArray("Allowed");
        for (int i = 0; i < count; ++i) {
            conf.setArrayIndex(i);
            QString host = conf.value("Host").toString();
            QSslCertificate certificate(conf.value("Certificate").toByteArray());
            allowedSslCertificates[host].insert(certificate);
        }
        conf.endArray();
    }

    void NetworkAccessManagerPrivate::getForBlocker(const QNetworkRequest & request, NetworkReplyBlocker * blocker)
    {
        QNetworkReply * reply = manager->get(request);
        connect(reply, SIGNAL(finished()), blocker, SLOT(quit()));
    }




    NetworkReplyBlocker::NetworkReplyBlocker(QObject * parent)
        : QEventLoop(parent), _mutex(QMutex::Recursive)
    {}

    NetworkReplyBlocker::~NetworkReplyBlocker()
    {}

    int NetworkReplyBlocker::exec(QEventLoop::ProcessEventsFlags flags)
    {
        QMutexLocker guard(&_mutex);
        if (!reply()) {
            return QEventLoop::exec(flags);
        }
        return 0;
    }

    QNetworkReply * NetworkReplyBlocker::reply() const
    {
        QMutexLocker guard(&_mutex);
        return _reply;
    }

    void NetworkReplyBlocker::quit()
    {
        QMutexLocker guard(&_mutex);
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        _reply = reply;
        QEventLoop::quit();
    }




    NetworkAccessManager::NetworkAccessManager(QObject * parent)
        : QNetworkAccessManager(parent), d(new NetworkAccessManagerPrivate(this))
    {
        qRegisterMetaType< QNetworkProxy >("QNetworkProxy");
        d->timeoutMapper = new QSignalMapper(this);
        connect(d->timeoutMapper, SIGNAL(mapped(QObject*)), this, SLOT(on_timeout(QObject*)));
        connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
                globalProxyFactory(), SLOT(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
                (thread() == globalProxyFactory()->thread() ? Qt::AutoConnection : Qt::BlockingQueuedConnection));
    }

    NetworkAccessManager::~NetworkAccessManager()
    {}

    QNetworkReply *	NetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
    {
        QNetworkRequest request(req);
        if (!request.hasRawHeader("User-Agent")) {
            request.setRawHeader("User-Agent", userAgentString().toLatin1());
        }

        QNetworkReply *	reply = QNetworkAccessManager::createRequest(op, request, outgoingData);
        connect(reply, SIGNAL(finished()), this, SLOT(on_finished()));
        connect(reply, SIGNAL(sslErrors(const QList< QSslError > &)), this, SLOT(on_sslErrors(const QList< QSslError > &)));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(on_downloadProgress(qint64, qint64)));
        connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(on_uploadProgress(qint64, qint64)));

        QTimer * timer = new QTimer(reply);
        timer->setSingleShot(true);
        timer->setInterval(20000);
        d->timeoutMapper->setMapping(timer, reply);
        connect(timer, SIGNAL(timeout()), d->timeoutMapper, SLOT(map()));
        connect(this, SIGNAL(stopTimers()), timer, SLOT(stop()));
        connect(this, SIGNAL(restartTimers()), timer, SLOT(start()));
        if (!d->paused) {
            timer->start();
        }

        return reply;
    }

    bool NetworkAccessManager::event(QEvent * e) // for dynamic propery changes
    {
        if (e->type() == QEvent::DynamicPropertyChange) {
            if (QDynamicPropertyChangeEvent * e2 = dynamic_cast< QDynamicPropertyChangeEvent * >(e)) {
                if (e2->propertyName() == "__u_doNotTimeout") {
                    if (property("__u_doNotTimeout").toBool()) {
                        emit stopTimers();
                    } else {
                        emit restartTimers();
                    }
                }
            }
        }
        return QNetworkAccessManager::event(e);
    }

    QNetworkReply * NetworkAccessManager::getAndBlock(const QNetworkRequest & request)
    {
        NetworkReplyBlocker loop;
        qRegisterMetaType< NetworkReplyBlocker * >("NetworkReplyBlocker *");
        QMetaObject::invokeMethod(d, "getForBlocker", Qt::QueuedConnection,
                                  Q_ARG(const QNetworkRequest &, request),
                                  Q_ARG(NetworkReplyBlocker *, &loop));
        loop.exec();
        return loop.reply();
    }

    void NetworkAccessManager::on_downloadProgress(qint64 downloaded, qint64 total)
    {
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        if (reply) {
            QTimer * timer = qobject_cast< QTimer * >(d->timeoutMapper->mapping(reply));
            if (timer) {
                timer->start();
            }
        }
    }

    void NetworkAccessManager::on_finished()
    {
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        if (reply) {
            QTimer * timer = qobject_cast< QTimer * >(d->timeoutMapper->mapping(reply));
            if (timer) {
                timer->stop();
            }
        }
    }

    void NetworkAccessManager::on_sslErrors(const QList< QSslError > & errors)
    {
        QList< QSslError > reportable;
        //qDebug() << "SSL Error(s):";
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        if (reply) {
            reply->manager()->setProperty("__u_doNotTimeout", true);

            QString host = reply->request().url().host();
            foreach(const QSslError & error, errors) {
                if (!d->allowedSslCertificates.value(host).contains(error.certificate())) {
                    switch (error.error()) {
                    case QSslError::CertificateExpired:
                    case QSslError::CertificateNotYetValid:
                    case QSslError::SelfSignedCertificate:
                    case QSslError::SelfSignedCertificateInChain:
                    case QSslError::CertificateUntrusted:
                    case QSslError::HostNameMismatch:
                        reportable.append(error);
                        break;
                    default:
                        // All other errors are unrecoverable
                        qDebug() << "!!!! Unrecoverable SSL error:";
                        qDebug() << "!!!!" << error.error();
                        reply->manager()->setProperty("__u_doNotTimeout", false);
                        return;
                    }
                }
            }

            if (reportable.isEmpty()) {
                reply->ignoreSslErrors();
            } else {
                CertificateErrorDialog dialog(host, reportable);
                dialog.exec();
                switch (dialog.allowPolicy()) {
                case CertificateErrorDialog::AllowAlways:
                    { // Store in configuration
                        QSettings conf;
                        conf.beginGroup("Networking");
                        conf.beginGroup("Security");
                        conf.beginWriteArray("Allowed");
                        foreach (QSslError error, reportable) {
                            conf.setArrayIndex(conf.childGroups().size());
                            conf.setValue("Host", host);
                            conf.setValue("Certificate", error.certificate().toPem());
                        }
                        conf.endArray();
                    }
                case CertificateErrorDialog::AllowSession:
                    foreach (QSslError error, reportable) {
                        d->allowedSslCertificates[host].insert(error.certificate());
                    }
                    reply->ignoreSslErrors();
                    break;
                case CertificateErrorDialog::Deny:
                default:
                    break;
                }
            }
            reply->manager()->setProperty("__u_doNotTimeout", false);
        }
    }

    void NetworkAccessManager::on_timeout(QObject * object)
    {
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(object);
        qDebug() << "TIMEOUT" << reply;
        if (reply) {
            if (property("__u_doNotTimeout").toBool()) {
                QTimer * timer = qobject_cast< QTimer * >(d->timeoutMapper->mapping(reply));
                qDebug() << "      :" << timer;
                if (timer) {
                    timer->start();
                }
            } else {
                reply->abort();
            }
        }
    }

    void NetworkAccessManager::on_uploadProgress(qint64, qint64)
    {
        QNetworkReply * reply = qobject_cast< QNetworkReply * >(sender());
        if (reply) {
            QTimer * timer = qobject_cast< QTimer * >(d->timeoutMapper->mapping(reply));
            if (timer) {
                timer->start();
            }
        }
    }

    void NetworkAccessManager::setUserAgentString(const QString & userAgentString)
    {
        d->userAgentString = userAgentString;
    }

    QString NetworkAccessManager::userAgentString() const
    {
        return d->userAgentString;
    }




    boost::shared_ptr< NetworkAccessManager > NetworkAccessManagerMixin::networkAccessManager() const
    {
        /*
        static QThreadStorage< boost::weak_ptr< NetworkAccessManager > * > singleton;
        if (!singleton.hasLocalData()) {
            singleton.setLocalData(new boost::weak_ptr< NetworkAccessManager >);
        }
        boost::shared_ptr< NetworkAccessManager > shared = *singleton.localData();
        if (singleton.localData()->isNull())
        {
            shared = boost::shared_ptr< NetworkAccessManager >(new NetworkAccessManager());
            *singleton.localData() = shared;
            _networkAccessManager = shared;
        }
        */

        static boost::weak_ptr< NetworkAccessManager > singleton;
        boost::shared_ptr< NetworkAccessManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< NetworkAccessManager >(new NetworkAccessManager());
            singleton = shared;
            _networkAccessManager = shared;
        }
        return shared;
    }

}
