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

#ifndef PAPYRO_PAPYROTAB_H
#define PAPYRO_PAPYROTAB_H

#include <spine/Document.h>

#include <QList>
#include <QString>
#include <QUrl>
#include <QFrame>
#include <QVariantMap>

class QMenu;
class QNetworkAccessManager;

namespace Utopia
{
    class Bus;
}

namespace Papyro
{

    class DocumentView;
    class SelectionProcessorAction;

    class PapyroTabPrivate;
    class PapyroTab : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(QString title
                   READ title
                   WRITE setTitle
                   NOTIFY titleChanged)
        Q_PROPERTY(QUrl url
                   READ url
                   NOTIFY urlChanged)
        Q_PROPERTY(qreal progress
                   READ progress
                   NOTIFY progressChanged)
        Q_PROPERTY(PapyroTab::State state
                   READ state
                   NOTIFY stateChanged)
        Q_PROPERTY(QString error
                   READ error
                   NOTIFY errorChanged)

    public:
        enum ActionType {
            QuickSearch,
            QuickSearchNext,
            QuickSearchPrevious,
            ToggleImageBrowser,
            ToggleLookupBar,
            TogglePager,
            ToggleSidebar
        };

        enum State {
            EmptyState,
            DownloadingState,
            DownloadingErrorState,
            LoadingState,
            LoadingErrorState,
            ProcessingState,
            IdleState
        };

        PapyroTab(QWidget * parent = 0);
        ~PapyroTab();

        QAction * action(ActionType actionType) const;
        SelectionProcessorAction * activeSelectionProcessorAction() const;
        Utopia::Bus * bus() const;
        void clearActiveSelectionProcessorAction();
        void clear();
        Spine::DocumentHandle document();
        DocumentView * documentView() const;
        QString error() const;
        bool isEmpty() const;
        QNetworkAccessManager * networkAccessManager() const;
        void open(Spine::DocumentHandle document, const QVariantMap & params = QVariantMap());
        void open(QIODevice * io, const QVariantMap & params = QVariantMap());
        void open(const QString & filename, const QVariantMap & params = QVariantMap());
        void open(const QUrl & url, const QVariantMap & params = QVariantMap());
        qreal progress() const;
        void setActiveSelectionProcessorAction(SelectionProcessorAction * processorAction = 0);
        void setSelectionProcessorActions(const QList< SelectionProcessorAction * > & processorActions);
        void setTitle(const QString & title);
        State state() const;
        QString title() const;
        QList< QAction * > toolActions() const;
        QUrl url() const;

    signals:
        void closeRequested();
        void contextMenuAboutToShow(QMenu * menu);
        void documentChanged();
        void errorChanged(const QString & error);
        void loadingChanged(bool loading);
        void progressChanged(qreal progress);
        void stateChanged(PapyroTab::State state);
        void titleChanged(const QString & title);
        void urlChanged(const QUrl & url);
        void urlRequested(const QUrl & url, const QString & target);

    public slots:
        void copySelectedText();
        void exploreSelection();
        void publishChanges();
        void quickSearch();
        void quickSearchNext();
        void quickSearchPrevious();
        void requestUrl(const QUrl & url, const QString & target = QString());

    protected:
        void closeEvent(QCloseEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);
        void setProgress(qreal progress);
        void setUrl(const QUrl & url);

    private:
        PapyroTabPrivate * d;
        friend class PapyroTabPrivate;
    }; // class PapyroTab

} // namespace Papyro

#endif // PAPYRO_PAPYROTAB_H
