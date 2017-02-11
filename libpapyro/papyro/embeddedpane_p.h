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

#ifndef PAPYRO_EMBEDDEDPANE_P_H
#define PAPYRO_EMBEDDEDPANE_P_H

#include <papyro/embeddedpane.h>
#include <utopia2/networkaccessmanager.h>
#include <QObject>
#include <QNetworkReply>
#include <QSharedPointer>

namespace Papyro
{

    class EmbeddedPanePrivate : public QObject, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        EmbeddedPanePrivate(EmbeddedPane * pane, EmbeddedPane::InteractionFlags flags);
        ~EmbeddedPanePrivate();

        EmbeddedPane * pane;
        EmbeddedPane::InteractionFlags flags;

        // Data used by subclasses
        QVariant data;

        // Keep track of status
        EmbeddedPane::Status status;
        QSharedPointer< QNetworkReply > reply;
        QString error;

        void setStatus(EmbeddedPane::Status status);

    signals:
        void statusChanged(EmbeddedPane::Status status);

    public slots:
        void startDownload(const QUrl & url);

    protected slots:
        void onNetworkReplyDownloadProgress(qint64 progress, qint64 total);
        void onNetworkReplyError(QNetworkReply::NetworkError code);
        void onNetworkReplyFinished();

    signals:
        void downloadAborted();
        void downloadFailed();
        void downloadComplete(QNetworkReply * reply);

    }; // class EmbeddedPanePrivate

} // namespace Papyro

#endif // PAPYRO_EMBEDDEDPANE_P_H