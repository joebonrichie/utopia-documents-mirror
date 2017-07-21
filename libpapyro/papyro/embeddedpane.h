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

#ifndef PAPYRO_EMBEDDEDPANE_H
#define PAPYRO_EMBEDDEDPANE_H

#include <QWidget>

class QNetworkReply;

namespace Papyro
{

    class EmbeddedPanePrivate;
    class EmbeddedPane : public QWidget
    {
        Q_OBJECT
        Q_ENUMS(Status)

    public:
        // What is the current status of this pane
        enum Status {
            Empty = 0,
            Downloading,
            DownloadError,
            Parsing,
            ParseError,
            Ready,
            Loading,
            LoadError,
            Active
        };

        // What sort of actions are allowed on this pane?
        enum InteractionFlag {
            Hidden          = 0x00,
            Embeddable      = 0x01, // Play/Pause button
            Launchable      = 0x02, // Launch button

            DefaultFlags    = Embeddable | Launchable
        };
        Q_DECLARE_FLAGS(InteractionFlags, InteractionFlag);

        EmbeddedPane(InteractionFlags flags = DefaultFlags, QWidget * parent = 0);

        QVariant data() const;
        QString error() const;
        InteractionFlags interactionFlags() const;
        void setData(const QVariant & data);
        void setInteractionFlags(InteractionFlags flags);
        Status status() const;

    protected slots:
        void onStatusChanged(EmbeddedPane::Status status);

    protected:
        EmbeddedPanePrivate * d;

        void showEvent(QShowEvent * event);

        void startDownload(const QUrl & url);
        void skipDownload();

        // These should be overriden in subclasses to do the actual work of
        // downloading / loading data
        virtual void download();
        virtual void load();
        virtual QVariant parseDownload(QNetworkReply * reply);

    }; // class EmbeddedPane

} // namespace Papyro

Q_DECLARE_OPERATORS_FOR_FLAGS(Papyro::EmbeddedPane::InteractionFlags);
Q_DECLARE_METATYPE(Papyro::EmbeddedPane::Status);

#endif // PAPYRO_EMBEDDEDPANE_H
