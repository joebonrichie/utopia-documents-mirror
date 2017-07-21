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

#ifndef PAPYRO_PAPYROTAB_P_H
#define PAPYRO_PAPYROTAB_P_H

#include <papyro/annotator.h>
#include <papyro/annotatorrunnablepool.h>
#include <papyro/decorator.h>
#include <papyro/documentmanager.h>
#include <papyro/librarymodel.h>
#include <papyro/papyrotab.h>
#include <papyro/citation.h>

#include <utopia2/bus.h>
#include <utopia2/busagent.h>
#include <utopia2/networkaccessmanager.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <boost/shared_ptr.hpp>
#endif

#include <QObject>
#include <QQueue>
#include <QSignalMapper>
#include <QString>
#include <QSvgRenderer>
#include <QTime>
#include <QTimer>
#include <QUrl>

class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QVBoxLayout;
class QSplitter;
class QStackedLayout;

namespace Utopia
{
    class FlowBrowser;
    class FlowBrowserModel;
    class Spinner;
} // namespace Utopia

namespace Papyro
{
    class AnnotationProcessor;
    class Dispatcher;
    class DocumentView;
    class DocumentProxy;
    class PageView;
    class Pager;
    class SearchBar;
    class SelectionProcessor;
    class SelectionProcessorFactory;
    class Sidebar;




    class WidgetExpander : public QWidget
    {
        Q_OBJECT

    public:
        WidgetExpander(QWidget * child, QWidget * parent);

        QWidget * child() const;
        int period() const;
        void setPeriod(int period);
        QSize sizeHint() const;

    protected:
        bool event(QEvent * event);
        void paintEvent(QPaintEvent * event);
        void resizeEvent(QResizeEvent * event);

    protected Q_SLOTS:
        void animate();
        void close();

    private:
        QTime _time;
        QTimer _timer;
        QWidget * _child;
        int _period;
        bool _expanding;
        int _oldHeight;
        QSize _childSizeHint;
    };




    class PapyroTabPrivate : public QObject, public Utopia::BusAgent, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        PapyroTabPrivate(PapyroTab * tab);
        ~PapyroTabPrivate();

        PapyroTab * tab;

        // This tab's general information
        QString title;
        QUrl url;
        qreal progress;
        QString error;
        PapyroTab::State state;
        Athenaeum::CitationHandle citation;

        // This tab's widgets
        //
        //   |---------------------------------------------------|
        //   | imageBrowser                     | sidebar        |
        //   |----------------------------------|                |
        //   |   | quickSearchBar           |   |                |
        //   |   \--------------------------/   |                |
        //   | documentView                     |                |
        //   |                                  |                |
        //   |                                  |                |
        //   |                                  |                |
        //   |                                  |                |
        //   |--------------------------------\ |                |
        //   | pager                          | | /--------------|
        //   |                                | | | lookupWidget |
        //   |---------------------------------------------------|
        //
        DocumentView * documentView;
        SearchBar * quickSearchBar;
        Utopia::FlowBrowser * imageBrowser;
        QWidget * lookupWidget;
        QLineEdit * lookupTextBox;
        QPushButton * lookupButton;
        Pager * pager;
        Sidebar * sidebar;
        QWidget * progressWidget;
        QSplitter * contentSplitter;

        // Progress widgets
        QStackedLayout * mainLayout;
        QLabel * progressIconLabel;
        QLabel * progressInfoLabel;
        QLabel * progressUrlLabel;
        QLabel * progressLinksLabel;
        Utopia::Spinner * progressSpinner;
        QSvgRenderer watermarkRenderer;
        void setError(const QString & reason);
        void setProgressMsg(const QString & msg, const QUrl & url);
        void setState(PapyroTab::State state);

        // Interaction
        QMap< PapyroTab::ActionType, QAction * > actions;
        QAction * actionQuickSearch;
        QAction * actionQuickSearchNext;
        QAction * actionQuickSearchPrevious;
        QAction * actionToggleImageBrowser;
        QAction * actionTogglePager;
        QAction * actionToggleSidebar;
        QAction * actionToggleLookupBar;

        // Management of the document
        boost::shared_ptr< DocumentManager > documentManager;
        DocumentProxy * documentProxy;
        boost::shared_ptr< Athenaeum::LibraryModel > libraryModel;

        // Management of the flow browser
        Utopia::FlowBrowserModel * imageBrowserModel;
        Utopia::FlowBrowserModel * chemicalBrowserModel;
        QList< Spine::Area > imageAreas;
        QList< Spine::TextExtentHandle > chemicalExtents;

        // Management of the page
        QTimer pagerTimer;
        QQueue< int > pagerQueue;
        QMap< int, int > areaAnnotationCountByPage;
        QMap< int, int > textAnnotationCountByPage;

        // Dispatching annotation / lookup jobs
        Dispatcher * dispatcher;
        QList< boost::shared_ptr< Annotator > > annotators;
        QList< boost::shared_ptr< Annotator > > activatableAnnotators;
        QList< boost::shared_ptr< Annotator > > lookups;
        QMap< QString, QMap< int, QList< boost::shared_ptr< Annotator > > > > eventHandlers;
        AnnotatorRunnablePool annotatorPool;
        int activeAnnotators;
        QSignalMapper * annotatorMapper;
        QList< Decorator * > decorators;

        // Handy debug overlay showing running / queued annotators (remove?)
        QVBoxLayout * statusLayout;
        Qt::Alignment statusAlignment;
        QTimer statusWidgetTimer;

        // Selection / Annotation processors
        QList< AnnotationProcessor * > annotationProcessors;
        QList< SelectionProcessorFactory * > selectionProcessorFactories;
        QList< SelectionProcessorAction * > selectionProcessorActions;
        SelectionProcessorAction * activeSelectionProcessorAction;

        // Annotators
        void open(Spine::DocumentHandle document,
                  const QVariantMap & params = QVariantMap(),
                  Athenaeum::CitationHandle citation = Athenaeum::CitationHandle());
        void queueAnnotatorRunnable(AnnotatorRunnable * runnable);
        bool ready;
        void loadAnnotators();
        void reloadAnnotators();
        void unloadAnnotators();

        // Helpful accessors
        Spine::DocumentHandle document() const;

    signals:
        void cancellationRequested();
        void closeRequested();
        void contextMenuAboutToShow(QMenu * menu);
        void errorChanged(const QString & reason);
        void stateChanged(PapyroTab::State state);
        void starredChanged(bool starred);
        void knownChanged(bool known);

    public slots:
        void cancelRunnables();

        // Annotation framework
        bool on_load_event_chain();
        bool on_activate_event_chain(boost::shared_ptr< Annotator > annotator = boost::shared_ptr< Annotator >(), const QVariantMap & kwargs = QVariantMap(), QObject * obj = 0, const char * receiver = 0);
        bool on_marshal_event_chain(QObject * obj = 0, const char * receiver = 0);

        // Event framework
        bool handleEvent(const QString & event, const QVariantMap & kwargs = QVariantMap(), QObject * obj = 0, const char * receiver = 0);
        bool handleEvent(boost::shared_ptr< Annotator > annotator, const QString & event, const QVariantMap & kwargs = QVariantMap(), QObject * obj = 0, const char * receiver = 0);

        // General management slots
        void activateChemicalImage(int i);
        void activateImage(int i);
        void executeAnnotator(int);
        void explore(const QString & term);
        void exploreSelection();
        void focusChanged(PageView * pageView, QPointF pagePos);
        void loadChemicalImage(int i);
        void loadImage(int i);
        void loadNextPagerImage();
        void onAnnotatorFinished();
        void onAnnotatorSkipped();
        void onAnnotatorStarted();
        void onCitationChanged(int index, QVariant value);
        void onDispatcherAnnotationFound(Spine::AnnotationHandle annotation);
        void onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void onDocumentAreaSelectionChanged(const std::string & name, const Spine::AreaSet & extents, bool added);
        void onDocumentProcessingFinished();
        void onDocumentTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added);
        void onDocumentViewAnnotationsActivated(Spine::AnnotationSet annotations, const QVariantMap & context);
        void onDocumentViewContextMenu(QMenu * menu, Spine::DocumentHandle document, Spine::CursorHandle cursor);
        //void onDocumentViewManageSelection(Spine::TextSelection selection, bool expand = false);
        //void onDocumentViewManageSelection(Spine::AreaSet areas);
        void onDocumentViewPageFocusChanged(size_t pageNumber);
        void onDocumentViewSpotlightsHidden();
        void onFilterFinished();
        void onImageBrowserEmptinessChanged(bool empty);
        void onLookupOverride();
        void onLookupStarted();
        void onLookupStopped();
        void onNetworkReplyFinished();
        void onNetworkReplyDownloadProgress(qint64, qint64);
        void onPagerPageClicked(int index);
        void onProgressLinksLabelLinkActivated(const QString & link);
        void onQuickSearchBarSearchForText(QString text);
        void onQuickSearchBarPrevious();
        void onQuickSearchBarNext();
        void onSidebarSelectionChanged();
        void onRemoveAnnotation(Spine::AnnotationHandle annotation);
        void publishChanges();
        void requestImage(int index);
        void showPager(bool show);
        void showImageBrowser(bool show);
        void showSidebar(bool show);
        void showLookupBar(bool show);
        void visualiseAnnotations(Spine::AnnotationSet annotations, const QVariantMap & context);

    protected:
        bool eventFilter(QObject * obj, QEvent * event);
        void resubscribeToBus();
        QString busId() const;
        void receiveFromBus(const QString & sender, const QVariant & data);

    }; // class PapyroTabPrivate

} // namespace Papyro

#endif // PAPYRO_PAPYROTAB_P_H
