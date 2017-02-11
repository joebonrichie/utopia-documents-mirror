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

#ifndef PAPYRO_PAPYROTAB_H
#define PAPYRO_PAPYROTAB_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Document.h>
#endif

#include <papyro/citation.h>

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
        Q_PROPERTY(bool starred
                   READ isStarred
                   NOTIFY starredChanged)
        Q_PROPERTY(bool known
                   READ isKnown
                   NOTIFY knownChanged)
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
            UninitialisedState,
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
        Athenaeum::CitationHandle citation() const;
        void clearActiveSelectionProcessorAction();
        void clear();
        Spine::DocumentHandle document();
        DocumentView * documentView() const;
        QString error() const;
        bool isEmpty() const;
        bool isKnown() const;
        bool isStarred() const;
        QNetworkAccessManager * networkAccessManager() const;
        void open(Spine::DocumentHandle document, const QVariantMap & params = QVariantMap(), Athenaeum::CitationHandle citation = Athenaeum::CitationHandle());
        void open(QIODevice * io, const QVariantMap & params = QVariantMap(), Athenaeum::CitationHandle citation = Athenaeum::CitationHandle());
        void open(const QString & filename, const QVariantMap & params = QVariantMap(), Athenaeum::CitationHandle citation = Athenaeum::CitationHandle());
        void open(const QUrl & url, const QVariantMap & params = QVariantMap(), Athenaeum::CitationHandle citation = Athenaeum::CitationHandle());
        //void open(const QVariantMap & citation, const QVariantMap & params = QVariantMap());
        void open(Athenaeum::CitationHandle citation, const QVariantMap & params = QVariantMap());
        qreal progress() const;
        void setActiveSelectionProcessorAction(SelectionProcessorAction * processorAction = 0);
        void setCitation(Athenaeum::CitationHandle citation);
        void setKnown(bool known);
        void setSelectionProcessorActions(const QList< SelectionProcessorAction * > & processorActions);
        void setStarred(bool starrred);
        void setTitle(const QString & title);
        State state() const;
        QString title() const;
        QList< QAction * > toolActions() const;
        QUrl url() const;

    signals:
        void citationsActivated(const QVariantList & citation, const QString & target);
        void closeRequested();
        void contextMenuAboutToShow(QMenu * menu);
        void citationChanged();
        void documentChanged();
        void errorChanged(const QString & error);
        void knownChanged(bool known);
        void starredChanged(bool starred);
        void loadingChanged(bool loading);
        void progressChanged(qreal progress);
        void stateChanged(PapyroTab::State state);
        void titleChanged(const QString & title);
        void urlChanged(const QUrl & url);
        void urlRequested(const QUrl & url, const QString & target);

    public slots:
        void addToLibrary();
        void copySelectedText();
        void exploreSelection();
        void publishChanges();
        void quickSearch();
        void quickSearchNext();
        void quickSearchPrevious();
        void removeFromLibrary();
        void requestUrl(const QUrl & url, const QString & target = QString());
        void star();
        void unstar();
        void visualiseAnnotations(const Spine::AnnotationSet & annotations);

    protected:
        void closeEvent(QCloseEvent * event);
        void resizeEvent(QResizeEvent * event);
        void setProgress(qreal progress);
        void setUrl(const QUrl & url);

    private:
        PapyroTabPrivate * d;
        friend class PapyroTabPrivate;
    }; // class PapyroTab

} // namespace Papyro

#endif // PAPYRO_PAPYROTAB_H
