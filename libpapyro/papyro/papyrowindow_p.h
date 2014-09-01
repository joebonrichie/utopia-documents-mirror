/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#ifndef PAPYROWINDOW_P_H
#define PAPYROWINDOW_P_H

#include <papyro/uimodifier.h>
#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <athenaeum/abstractbibliographiccollection.h>
#include <athenaeum/bibliographicsearchbox.h>
#include "utopia2/qt/abstractwindow_p.h"
#include <utopia2/networkaccessmanager.h>

#include <boost/shared_ptr.hpp>

#include <QModelIndex>
#include <QObject>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QQueue>
#include <QRunnable>
#include <QScopedPointer>
#include <QSignalMapper>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QThreadPool>
#include <QWeakPointer>
#include <QWidget>
#include <QUrl>

#include <QEvent>
#include <QtDebug>

class QAction;
class QButtonGroup;
class QFrame;
class QItemSelection;
class QLabel;
class QLineEdit;
class QMenu;
class QMimeData;
class QModelIndex;
class QPaintEvent;
class QProgressDialog;
class QPushButton;
class QSignalMapper;
class QSplitter;
class QSplitterHandle;
class QStackedLayout;
class QPropertyAnimation;
class QTabBar;
class QToolButton;
class QVBoxLayout;

namespace Spine
{
    class Document;
}

namespace Utopia
{
    class BubbleWidget;
    class FlowBrowser;
    class FlowBrowserModel;
    class HoldableButton;
    class ProgressDialog;
    class Spinner;
}

namespace Kend
{
    class ServiceStatusIcon;
}

namespace Athenaeum
{
    class AbstractFilter;
    class AggregatingProxyModel;
    class ArticleView;
    class Exporter;
    class LibraryModel;
    class RemoteQueryBibliographicModel;
    class Resolver;
    class SortFilterProxyModel;
}

namespace Papyro
{

    class AnnotationProcessor;
    class Decorator;
    class Dispatcher;
    class DocumentSignalProxy;
    class HelpWidget;
    class Pager;
    class PapyroRecentUrlHelper;
    class PapyroWindow;
    class Printer;
    class SearchBar;
    class SelectionManager;
    class SelectionProcessor;
    class SelectionProcessorAction;
    class Sidebar;
    class TabBar;




    class PapyroWindowPrivate : public Utopia::AbstractWindowPrivate, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        // Set up manager transparently
        PapyroWindowPrivate(PapyroWindow * publicObject);
        ~PapyroWindowPrivate();

        PapyroWindow * window();
        const PapyroWindow * window() const;

        // General UI
        //QStatusBar * statusBar;
        //QLabel * statusLabel;
        //Utopia::Spinner * spinner;

        // Actions
        QAction * actionAdd;
        QAction * actionOpen;
        QAction * actionOpenUrl;
        QAction * actionOpenFromClipboard;
        QAction * actionSave;
        QAction * actionPrint;
        QAction * actionClose;
        QAction * actionQuit;

        QAction * actionCopy;

        QAction * actionNextTab;
        QAction * actionPreviousTab;
        QAction * actionShowSearch;
        QAction * actionShowDocuments;
#ifdef UTOPIA_BUILD_DEBUG
        QAction * actionShowLibrary;
#endif

        QAction * actionShowHelp;
        QAction * actionShowAbout;

        boost::shared_ptr< PapyroRecentUrlHelper > recentUrlHelper;
        boost::shared_ptr< Printer > printer;

        // Menus
        QMenu * menuFile;
        QMenu * menuEdit;
        QMenu * menuView;

        // Layout
        QUrl url;
        QFrame * sliver;
        QFrame * cornerFrame;
        QToolButton * cornerButton;
        TabBar * tabBar;
        QStackedLayout * tabLayout;
        Athenaeum::BibliographicSearchBox * searchBox;

        // Remote searching / library
        Athenaeum::LibraryModel * libraryModel;
        Athenaeum::SortFilterProxyModel * filterProxyModel;
        Athenaeum::AggregatingProxyModel * aggregatingProxyModel;
        QMap< int, Athenaeum::AbstractFilter * > standardFilters;
        Athenaeum::ArticleView * articleResultsView;
        QMap< QString, Athenaeum::Exporter * > exporters;
        //QMap< int, QList< Athenaeum::Resolver * > > resolvers;
        QList< Athenaeum::RemoteQueryBibliographicModel * > remoteSearches;
        QFrame * remoteSearchLabelFrame;
        QLabel * remoteSearchLabel;
        Utopia::Spinner * remoteSearchLabelSpinner;
        void removeRemoteSearch();
        QWeakPointer< Utopia::BubbleWidget > articlePreviewBubble;
        QModelIndex articlePreviewIndex;
        QTimer articlePreviewTimer;

        QList< QUrl > checkForSupportedUrls(const QMimeData * mimeData);

        // Selection / Annotation processors
        //QList< AnnotationProcessor * > annotationProcessors;
        QList< SelectionProcessorAction * > selectionProcessorActions;
        SelectionProcessorAction * activePrimaryToolAction;
        QSignalMapper primaryToolSignalMapper;
        QButtonGroup * primaryToolButtonGroup;
        QIcon generateToolIcon(const QPixmap & pixmap, const QSize & size = QSize(24, 24));
        QIcon generateToolIcon(const QIcon & icon, const QSize & size = QSize(24, 24));
        QToolButton * addPrimaryToolButton(const QIcon & icon, const QString & text, int index, bool grouped = true);

        // Mode buttons
        DocumentView::InteractionMode interactionMode;
        Utopia::HoldableButton * selectingModeButton;
        Utopia::HoldableButton * highlightingModeButton;
        QColor highlightingColor;
        Utopia::HoldableButton * doodlingModeButton;
        QFrame * highlightingModeOptionFrame;
        void updateHighlightingModeButton();

        // Tab management
        QList< QPointer< QAction > > currentTabActions;
        void addTab(PapyroTab * tab);
        PapyroTab * currentTab() const;
        PapyroTab * emptyTab();
        PapyroTab * newTab();
        PapyroTab * tabAt(int index);
        QList< PapyroTab * > tabs();

        // Layers
        enum Layer { SearchLayer, SearchControlLayer, DocumentLayer, SliverLayer };
        QMap< Layer, QWidget * > layers;
        QParallelAnimationGroup layerAnimationGroup;
        QMap< Layer, QPropertyAnimation * > layerAnimations;
        enum LayerState { DocumentState, SearchState };
        QRect layerGeometry(Layer layer) const;
        QRect layerGeometryForState(Layer layer, LayerState layerState) const;
        LayerState toLayerState;
        LayerState currentLayerState;
        void changeToLayerState(LayerState layerState);
        void updateManualLayouts();

        // General UI / menus
        void initialise();
        void rebuildMenus();
        PapyroTab * takeTab(int index);
        void updateTabVisibility();

    signals:
        void currentTabChanged();

    public slots:
        void closeArticlePreview();
        void closeOtherTabs(int index);
        void closeTab(int index);
        void copySelectedText();
        void deleteSelectedArticles();
        void exportArticleCitations(const QItemSelection & selection);
        void exportCitationsOfSelectedArticles();
        void moveTabToNewWindow(int index);
        void onArticleActivated(const QModelIndex & index);
        void onArticlePreviewRequested(const QModelIndex & index);
        void onArticleViewCustomContextMenuRequested(const QPoint & pos);
        void onClipboardDataChanged();
        void onClose();
        void onCornerButtonClicked(bool checked);
        void onCurrentTabChanged(int index);
        void onFilterRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain searchDomain);
        void onHighlightingModeOptionsRequested();
        void onHighlightingModeOptionChosen();
        void onModeChange(int mode);
        void onModeChangeSelecting();
        void onModeChangeHighlighting();
        void onModeChangeDoodling();
        void onNewWindow();
        void onPrimaryToolButtonClicked(int idx);
        void onPrint();
        void onRemoteSearchStateChanged(Athenaeum::AbstractBibliographicCollection::State state);
        void onResolverRunnableCompleted(QModelIndex index, QVariantMap metadata);
        void onSearchRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain searchDomain);
        void onTabBarCustomContextMenuRequested(const QPoint & pos);
        void onTabContextMenu(QMenu * menu);
        void onTabDocumentChanged();
        void onTabLayoutChanged();
        void onTabStateChanged(PapyroTab::State state);
        void onTabTitleChanged(const QString & title);
        void onTabUrlChanged(const QUrl & url);
        void onUrlRequested(const QUrl & url, const QString & target);
        void openSelectedArticles();
        void showDocuments();
        void showLibrary();
        void showSearch();
        void updateTabInfo();

    protected:
        bool eventFilter(QObject * obj, QEvent * event);
    };


}

#endif // PAPYROWINDOW_P_H
