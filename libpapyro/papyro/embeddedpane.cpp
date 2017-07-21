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

#include <papyro/embeddedpane.h>
#include <papyro/embeddedpane_p.h>

#include <utopia2/networkaccessmanager.h>

namespace Papyro
{

    EmbeddedPanePrivate::EmbeddedPanePrivate(EmbeddedPane * pane, EmbeddedPane::InteractionFlags flags)
        : QObject(pane), pane(pane), flags(flags), status(EmbeddedPane::Empty)
    {
        qRegisterMetaType< EmbeddedPane::Status >("EmbeddedPane::Status");
    }

    EmbeddedPanePrivate::~EmbeddedPanePrivate()
    {}

    void EmbeddedPanePrivate::onNetworkReplyDownloadProgress(qint64 progress, qint64 total)
    {
    }

    void EmbeddedPanePrivate::onNetworkReplyError(QNetworkReply::NetworkError code)
    {
        if (reply && reply == static_cast< QNetworkReply * >(sender())) {
            QUrl url = reply->request().url();
            QString host = url.host();

            switch (code) {
            case QNetworkReply::ContentNotFoundError:
                error = "Requested data not available";
                break;
            case QNetworkReply::HostNotFoundError:
                error = "Host not found ("+host+")";
                break;
            case QNetworkReply::ConnectionRefusedError:
                error = "Connection refused ("+host+")";
                break;
            case QNetworkReply::RemoteHostClosedError:
                error = "Unexpected disconnection ("+host+")";
                break;
            case QNetworkReply::ProtocolFailure:
                error = "Malformed response ("+host+")";
                break;
            case QNetworkReply::ProxyAuthenticationRequiredError:
            case QNetworkReply::AuthenticationRequiredError:
                error = "Authentication failed ("+host+")";
                break;
            case QNetworkReply::OperationCanceledError:
            case QNetworkReply::TimeoutError:
                error = "Network timeout occurred";
                break;
            default:
                error = "Unknown data download error";
                break;
            }
            reply.reset();
        }

        setStatus(EmbeddedPane::DownloadError);
    }

    void EmbeddedPanePrivate::onNetworkReplyFinished()
    {
        if (reply && reply == static_cast< QNetworkReply * >(sender())) {
            QVariant redirectsVariant = reply->property("__redirects");
            QVariant originalData = reply->property("__originalData");

            // Deal with redirects correctly (up to 20 hops)
            int redirects = redirectsVariant.isNull() ? 20 : redirectsVariant.toInt();
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
                    reply.reset(networkAccessManager()->get(request), &QObject::deleteLater);
                    reply->setProperty("__redirects", redirects - 1);
                    reply->setProperty("__originalData", data);
                    connect(reply.data(), SIGNAL(finished()), this, SLOT(onNetworkReplyFinished()));
                    connect(reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkReplyError(QNetworkReply::NetworkError)));
                    connect(reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
                    return;
                } else {
                    // Too many times
                    error = "The document URL redirected too many times, so I've abandoned the attempt.";
                }
            } else if (reply->error() == QNetworkReply::NoError) {
                setStatus(EmbeddedPane::Parsing);
            }
        }
    }

    void EmbeddedPanePrivate::setStatus(EmbeddedPane::Status status)
    {
        if (status != this->status) {
            this->status = status;
            emit statusChanged(status);
        }
    }

    void EmbeddedPanePrivate::startDownload(const QUrl & url)
    {
        reply.reset(networkAccessManager()->get(QNetworkRequest(url)), &QObject::deleteLater);
        connect(reply.data(), SIGNAL(finished()), this, SLOT(onNetworkReplyFinished()));
        connect(reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkReplyError(QNetworkReply::NetworkError)));
        connect(reply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
    }





    EmbeddedPane::EmbeddedPane(InteractionFlags flags, QWidget * parent)
        : QWidget(parent), d(new EmbeddedPanePrivate(this, flags))
    {
        // Ensure status changes drive the pane
        connect(d, SIGNAL(statusChanged(EmbeddedPane::Status)),
                this, SLOT(onStatusChanged(EmbeddedPane::Status)),
                Qt::QueuedConnection);
    }

    QVariant EmbeddedPane::data() const
    {
        return d->data;
    }

    void EmbeddedPane::download()
    {
        // By default, download nothing, and skip to the next step
        skipDownload();
    }

    QString EmbeddedPane::error() const
    {
        return d->error;
    }

    EmbeddedPane::InteractionFlags EmbeddedPane::interactionFlags() const
    {
        return d->flags;
    }

    void EmbeddedPane::load()
    {
        // This method should be overidden in the subclass to load the data
        d->error = "No load method implemented";
        d->setStatus(LoadError);
    }

    QVariant EmbeddedPane::parseDownload(QNetworkReply * reply)
    {
        // This method should be overridden to take the data from the reply
        // and return it in a form that can then be loaded
        return QVariant();
    }

    void EmbeddedPane::onStatusChanged(Status status)
    {
        //qDebug() << "***** onStatusChanged to" << status;
        switch (status) {
        case Downloading:
            download();
            break;
        case Parsing: {
            QVariant parsed(parseDownload(d->reply.data()));
            if (parsed.isValid()) {
                setData(parsed);
                d->setStatus(Ready);
            } else {
                d->setStatus(ParseError);
            }
            break;
        }
        case Ready:
            d->setStatus(Loading);
            break;
        case Loading:
            load();
            d->setStatus(Active);
            break;
        default:
            // Do nothing?
            break;
        }
    }

    void EmbeddedPane::showEvent(QShowEvent * event)
    {
        if (status() == Empty) {
            d->setStatus(Downloading);
        }
    }

    void EmbeddedPane::skipDownload()
    {
        // Skip ahead
        d->setStatus(Ready);
    }

    void EmbeddedPane::startDownload(const QUrl & url)
    {
        if (url.isValid()) {
            d->startDownload(url);
        } else {
            d->error = "Cannot download data from invalid URL";
            d->setStatus(DownloadError);
        }
    }

    void EmbeddedPane::setData(const QVariant & data)
    {
        //qDebug() << "***** setData to" << data;
        d->data = data;
    }

    void EmbeddedPane::setInteractionFlags(InteractionFlags flags)
    {
        d->flags = flags;
    }

    EmbeddedPane::Status EmbeddedPane::status() const
    {
        return d->status;
    }

} // namespace Papyro
