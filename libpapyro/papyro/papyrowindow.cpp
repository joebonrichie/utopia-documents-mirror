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

#include "papyrowindow_p.h"
#include "papyrowindow.h"

#include <papyro/annotator.h>
#include <papyro/annotationresultitem.h>
#include <papyro/capabilities.h>
#include <papyro/cslengine.h>
#include <papyro/decorator.h>
#include <papyro/dispatcher.h>
#include <papyro/documentmanager.h>
#include <papyro/documentfactory.h>
#include <papyro/documentsignalproxy.h>
#include <papyro/documentview.h>
#include <papyro/pager.h>
#include <papyro/printer.h>
#include <papyro/sidebar.h>
#include <papyro/utils.h>
#include <papyro/resultsview.h>
#include <papyro/papyrotab.h>
#include <papyro/progresslozenge.h>
#include <papyro/raisetabaction.h>
#include <papyro/searchbar.h>
#include <papyro/selectionprocessor.h>
#include <papyro/selectionprocessoraction.h>
#include <papyro/sliver.h>
#include <papyro/tabbar.h>
#include <athenaeum/articledelegate.h>
#include <athenaeum/articleview.h>
#include <athenaeum/aggregatingproxymodel.h>
#include <athenaeum/bibliographicitem_p.h>
#include <athenaeum/exporter.h>
#include <athenaeum/filters.h>
#include <athenaeum/librarymodel.h>
#include <athenaeum/persistentbibliographicmodel.h>
#include <athenaeum/remotequery.h>
#include <athenaeum/remotequerybibliographicmodel.h>
#include <athenaeum/resolver.h>
#include <athenaeum/resolverrunnable.h>
#include <athenaeum/sortfilterproxymodel.h>
#include <spine/spine.h>
#include <utopia2/qt/aboutdialog.h>
#include <utopia2/qt/bubble.h>
#include <utopia2/qt/filedialog.h>
#include <utopia2/qt/flowbrowser.h>
#include <utopia2/qt/holdablebutton.h>
#include <utopia2/qt/progressdialog.h>
#include <utopia2/qt/raiseaction.h>
#include <utopia2/qt/spinner.h>
#include <utopia2/qt/uimanager.h>
#include <utopia2/auth/qt/servicestatusicon.h>
#include <utopia2/fileformat.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsBlurEffect>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QIODevice>
#include <QKeyEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMetaType>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QPushButton>
#include <QQueue>
#include <QScrollBar>
#include <QSettings>
#include <QSignalMapper>
#include <QSplitter>
#include <QSplitterHandle>
#include <QStackedLayout>
#include <QStatusBar>
#include <QPropertyAnimation>
#include <QTextEdit>
#include <QTextDocument>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Papyro
{

    /// PapyroWindowPrivate /////////////////////////////////////////////////////////////


    PapyroWindowPrivate::PapyroWindowPrivate(PapyroWindow * publicObject)
        : Utopia::AbstractWindowPrivate(publicObject), interactionMode(DocumentView::SelectingMode), highlightingColor(Qt::yellow)
    {
        recentUrlHelper = PapyroRecentUrlHelper::instance();
        printer = Printer::instance();

        QObject::connect(uiManager.get(), SIGNAL(activateRecentUrl(QUrl)), recentUrlHelper.get(), SLOT(activateRecentUrl(QUrl)), Qt::UniqueConnection);

        connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));

        articlePreviewTimer.setInterval(400);
        articlePreviewTimer.setSingleShot(true);
        connect(&articlePreviewTimer, SIGNAL(timeout()), this, SLOT(closeArticlePreview()));
    }

    PapyroWindowPrivate::~PapyroWindowPrivate()
    {
    }

    QToolButton * PapyroWindowPrivate::addPrimaryToolButton(const QIcon & icon, const QString & text, int index, bool grouped)
    {
        QAction * action = new QAction(generateToolIcon(icon), text, primaryToolButtonGroup);
        if (grouped) action->setCheckable(true);
        primaryToolSignalMapper.setMapping(action, index);
        connect(action, SIGNAL(triggered()), &primaryToolSignalMapper, SLOT(map()));
        QToolButton * button = new QToolButton;
        if (grouped) primaryToolButtonGroup->addButton(button);
        button->setDefaultAction(action);
        sliver->layout()->addWidget(button);
        sliver->layout()->setAlignment(button, Qt::AlignHCenter);
        return button;
    }

    void PapyroWindowPrivate::addTab(PapyroTab * tab)
    {
        tab->setSelectionProcessorActions(selectionProcessorActions);
        tab->setActiveSelectionProcessorAction(activePrimaryToolAction);
        connect(tab, SIGNAL(closeRequested()), this, SLOT(onClose()));
        connect(tab, SIGNAL(documentChanged()), this, SLOT(onTabDocumentChanged()));
        connect(tab, SIGNAL(stateChanged(PapyroTab::State)), this, SLOT(onTabStateChanged(PapyroTab::State)));
        connect(tab, SIGNAL(titleChanged(const QString &)), this, SLOT(onTabTitleChanged(const QString &)));
        connect(tab, SIGNAL(urlChanged(const QUrl &)), this, SLOT(onTabUrlChanged(const QUrl &)));
        connect(tab, SIGNAL(urlRequested(const QUrl &, const QString &)), this, SLOT(onUrlRequested(const QUrl &, const QString &)));
        connect(tab, SIGNAL(contextMenuAboutToShow(QMenu *)), this, SLOT(onTabContextMenu(QMenu *)));
        tabLayout->addWidget(tab);
        tabBar->addTab(tab);
    }

    void PapyroWindowPrivate::changeToLayerState(LayerState layerState)
    {
        if (toLayerState != layerState) {
            toLayerState = layerState;

            if (QWidget * layerWidget = layers.value(SearchLayer, 0)) {
                layerWidget->setEnabled(toLayerState == SearchState);
            }
            // FIXME not sure if this should be disabled, so removing its disabling code
            //if (QWidget * layerWidget = layers.value(DocumentLayer, 0)) {
            //    layerWidget->setEnabled(toLayerState == DocumentState);
            //}

            // Stop all animations dead and make sure their ranges are set correctly
            layerAnimationGroup.stop();
            QMutableMapIterator< PapyroWindowPrivate::Layer, QPropertyAnimation * > iter(layerAnimations);
            while (iter.hasNext()) {
                iter.next();
                iter.value()->setStartValue(layerGeometry(iter.key()));
                iter.value()->setEndValue(layerGeometryForState(iter.key(), layerState));
            }
            QTimer::singleShot(0, &layerAnimationGroup, SLOT(start()));

            // Make sure the right things are focused / shown
            switch (toLayerState) {
            case SearchState:
                searchBox->setFocus();
                cornerButton->setChecked(true);
                break;
            default:
                cornerButton->setChecked(false);
                break;
            }
        }
    }

    QList< QUrl > PapyroWindowPrivate::checkForSupportedUrls(const QMimeData * mimeData)
    {
        QSet< QUrl > urls;
        if (mimeData) {
            foreach (const QUrl & url, mimeData->urls()) {
                urls << url;
            }
            if (urls.isEmpty()) {
                QString text(mimeData->text());
                if (text.indexOf(QRegExp("[a-zA-Z]+://")) == 0) {
                    QUrl textUrl(QUrl::fromEncoded(text.toAscii()));
                    if (!textUrl.isValid()) {
                        textUrl = QUrl(text);
                    }
                    if (textUrl.isValid()) {
                        urls << textUrl;
                    }
                }
            }
        }
        return urls.toList();
    }

    void PapyroWindowPrivate::closeArticlePreview()
    {
        if (articlePreviewBubble) {
            articlePreviewBubble.data()->hide();
            articlePreviewBubble.data()->deleteLater();
            articlePreviewBubble.clear();
        }
        articlePreviewIndex = QModelIndex();
        articlePreviewTimer.stop();
    }

    void PapyroWindowPrivate::closeOtherTabs(int index)
    {
        for (int i = tabBar->count() - 1; i >= 0; --i) {
            if (index != i) {
                closeTab(i);
            }
        }
    }

    void PapyroWindowPrivate::closeTab(int index)
    {
        if (PapyroTab * tab = tabAt(index)) {
            bool wasEmpty = tab->state() == PapyroTab::EmptyState;
            if (tab->close()) {
                takeTab(index);
                tab->deleteLater();

                // if the closed tab was empty, close the window
                // FIXME what should really happen once the Library is behind?
                if (tabBar->isEmpty()) {
                    if (wasEmpty) {
                        window()->close();
                    } else {
                        emptyTab();
                    }
                }
            }
        }
    }

    void PapyroWindowPrivate::copySelectedText()
    {
        currentTab()->copySelectedText();
    }

    PapyroTab * PapyroWindowPrivate::currentTab() const
    {
        return qobject_cast< PapyroTab * >(tabLayout->currentWidget());
    }

    void PapyroWindowPrivate::deleteSelectedArticles()
    {
        QModelIndexList doomed = articleResultsView->selectionModel()->selectedIndexes();
        qSort(doomed.begin(), doomed.end(), qGreater< QModelIndex >());
        foreach (const QModelIndex & index, doomed) {
            articleResultsView->model()->removeRow(index.row());
        }
    }

    PapyroTab * PapyroWindowPrivate::emptyTab()
    {
        PapyroTab * tab = currentTab();
        if (tab && tabLayout->count() == 1 && tab->isEmpty()) {
            // FIXME when a tab is in error, should it be overwritten by a new open() ?
            // currently only a single tab is overwritten.
            return currentTab();
        } else {
            return newTab();
        }
    }

    bool PapyroWindowPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        //qDebug() << obj << event;
        if (obj == articlePreviewBubble.data() || obj == articleResultsView) {
            if (event->type() == QEvent::Leave) {
                articlePreviewTimer.start();
            } else if (event->type() == QEvent::Enter) {
                articlePreviewTimer.stop();
            }
        } else if (obj == articleResultsView->viewport()) {
            if (event->type() == QEvent::Wheel ||
                event->type() == QEvent::MouseButtonDblClick ||
                event->type() == QEvent::MouseButtonRelease) {
                articlePreviewTimer.stop();
                closeArticlePreview();
            }
        }
        return Utopia::AbstractWindowPrivate::eventFilter(obj, event);
    }

    void PapyroWindowPrivate::exportArticleCitations(const QItemSelection & selection)
    {
        // Get a filename for the citation export as well as which exporter to use
        QString s("s"), ies("ies");
        if (selection.size() == 1 && selection.at(0).height() == 1 && selection.at(0).width() == 1) {
            s = "";
            ies = "y";
        }

        // Compile information for file dialog
        QMap< QString, Athenaeum::Exporter * > formats;
        foreach (Athenaeum::Exporter * exporter, exporters.values()) {
            QString format = QString("%1 (*.%2)").arg(exporter->name(), exporter->extensions().join(" *."));
            formats[format] = exporter;
        }
        QString filter = QStringList(formats.keys()).join(";;");

        // Request an export filename from the user
        QString selectedFilter;
        QString filename = QFileDialog::getSaveFileName(window(), "Export selected article" + s, QString(), filter, &selectedFilter);
        Athenaeum::Exporter * exporter = formats.value(selectedFilter, 0);
        if (exporter && !filename.isEmpty()) {
            QModelIndexList indexes;
            foreach (const QItemSelectionRange & range, selection) {
                indexes += QItemSelectionRange(range.topLeft(), range.topLeft().sibling(range.bottom(), range.left())).indexes();
            }
            exporter->doExport(indexes, filename);
        }
    }

    void PapyroWindowPrivate::exportCitationsOfSelectedArticles()
    {
        exportArticleCitations(articleResultsView->selectionModel()->selection());
    }

    QIcon PapyroWindowPrivate::generateToolIcon(const QPixmap & pixmap, const QSize & size)
    {
        return generateToolIcon(QIcon(pixmap), size);
    }

    QIcon PapyroWindowPrivate::generateToolIcon(const QIcon & icon, const QSize & size)
    {
        QIcon fixed(icon);
        QPixmap iconOff(fixed.pixmap(size, QIcon::Normal, QIcon::Off));
        if (iconOff.isNull()) {
            // Default to question mark
            fixed.addPixmap(QPixmap(":/processors/default/icon.png"), QIcon::Normal, QIcon::Off);
        }
        QPixmap iconOn(fixed.pixmap(size, QIcon::Normal, QIcon::On));
        if (iconOn.isNull()) {
            // Default to question mark
            iconOn = QPixmap(":/processors/default/icon.png");
            fixed.addPixmap(iconOn, QIcon::Normal, QIcon::On);
        }
        if (!iconOn.isNull()) {
            QPainter painter(&iconOn);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            painter.setPen(QColor(0, 0, 0, 80));
            painter.setBrush(QColor(0, 0, 0, 20));
            painter.drawRect(iconOn.rect().adjusted(0, 0, -1, -1));
        }
        fixed.addPixmap(iconOn, QIcon::Normal, QIcon::On);
        return fixed;
    }

    void PapyroWindowPrivate::initialise()
    {
        // Get defaults
        QVariantMap defaults(Utopia::defaults());

        connect(this, SIGNAL(currentTabChanged()), window(), SIGNAL(currentTabChanged()));

        window()->setAutoFillBackground(true);
        window()->setWindowTitle("Utopia Documents");

        // Central widget
        QWidget * mainWidget = new QWidget;
        window()->setCentralWidget(mainWidget);
        const int layerAnimtationDuration = 200;

        foreach (Athenaeum::Exporter * exporter, Utopia::instantiateAllExtensions< Athenaeum::Exporter >()) {
            exporters[exporter->name()] = exporter;
        }

        foreach (Athenaeum::Resolver * resolver, Utopia::instantiateAllExtensions< Athenaeum::Resolver >()) {
            resolvers[resolver->weight()] << resolver;
        }

        /////////////////////////////////////////////////////////////////////////////////
        // Search layer

        QWidget * searchLayer = layers[SearchLayer] = new QWidget(mainWidget);
        searchLayer->setObjectName("search_layer");
        layerAnimationGroup.addAnimation(layerAnimations[SearchLayer] = new QPropertyAnimation(searchLayer, "geometry", this));
        layerAnimations[SearchLayer]->setEasingCurve(QEasingCurve::InOutSine);

        QVBoxLayout * searchLayout = new QVBoxLayout(searchLayer);
        {
            QMargins margins(searchLayout->contentsMargins());
            margins.setRight(0);
            margins.setBottom(0);
            searchLayout->setContentsMargins(margins);
        }

        {
            QLabel * label = new QLabel;
            label->setStyleSheet("font-size: 10pt; color: white; padding: 0px;");
            label->setText("Search for articles (in PubMed)");
            label->setAlignment(Qt::AlignCenter);
            searchLayout->addWidget(label, 0);
        }

        QFrame * remoteSearchHeaderFrame = new QFrame;
        remoteSearchHeaderFrame->setObjectName("remote_search_header");
        QVBoxLayout * searchHeaderLayout = new QVBoxLayout(remoteSearchHeaderFrame);
        searchHeaderLayout->setContentsMargins(0, 0, 0, 0);
        searchHeaderLayout->setSpacing(0);

        searchBox = new Athenaeum::BibliographicSearchBox;
        // FIXME filtering is disabled for the day-zero release
        //connect(searchBox, SIGNAL(filterRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)),
        //        this, SLOT(onFilterRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)));
        connect(searchBox, SIGNAL(searchRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)),
                this, SLOT(onSearchRequested(const QString &, Athenaeum::BibliographicSearchBox::SearchDomain)));
        searchHeaderLayout->addWidget(searchBox, 0);

        remoteSearchLabelFrame = new QFrame;
        remoteSearchLabelFrame->setObjectName("remote_search_label");
        QHBoxLayout * remoteSearchLabelFrameLayout = new QHBoxLayout(remoteSearchLabelFrame);
        remoteSearchLabelFrameLayout->setContentsMargins(0, 0, 0, 0);
        remoteSearchLabelFrameLayout->setSpacing(4);
        remoteSearchLabelFrameLayout->addWidget(remoteSearchLabel = new QLabel, 1);
        remoteSearchLabelFrameLayout->addWidget(remoteSearchLabelSpinner = new Utopia::Spinner, 0);
        remoteSearchLabelSpinner->setFixedSize(24, 24);
        remoteSearchLabelSpinner->setColor(QColor(255, 255, 255, 180));
        searchHeaderLayout->addWidget(remoteSearchLabelFrame, 0);

        articleResultsView = new Athenaeum::ArticleView;
        articleResultsView->setMouseTracking(true);
        articleResultsView->setDragEnabled(true);
        articleResultsView->setAcceptDrops(true);
        articleResultsView->setDropIndicatorShown(false);
        articleResultsView->setDefaultDropAction(Qt::MoveAction);
        articleResultsView->setItemDelegate(new Athenaeum::ArticleDelegate);
        articleResultsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        articleResultsView->horizontalScrollBar()->disconnect(); // no horizontal scrolling
        articleResultsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        articleResultsView->setAlternatingRowColors(false);
        articleResultsView->setFrameStyle(QFrame::NoFrame);
        articleResultsView->setAttribute(Qt::WA_MacShowFocusRect, false);
        articleResultsView->setDragDropMode(QAbstractItemView::DragDrop);
        articleResultsView->setContextMenuPolicy(Qt::CustomContextMenu);
        articleResultsView->installEventFilter(this);
        articleResultsView->viewport()->installEventFilter(this);
        connect(articleResultsView, SIGNAL(activated(const QModelIndex &)),
                this, SLOT(onArticleActivated(const QModelIndex &)));
        connect(articleResultsView, SIGNAL(clicked(const QModelIndex &)),
                this, SLOT(onArticlePreviewRequested(const QModelIndex &)));
        connect(articleResultsView, SIGNAL(previewRequested(const QModelIndex &)),
                this, SLOT(onArticlePreviewRequested(const QModelIndex &)));
        connect(articleResultsView, SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(onArticleViewCustomContextMenuRequested(const QPoint &)));

        {
            aggregatingProxyModel = new Athenaeum::AggregatingProxyModel(Qt::Vertical, this);
            filterProxyModel = new Athenaeum::SortFilterProxyModel(this);
            articleResultsView->setModel(filterProxyModel);
            standardFilters[Athenaeum::BibliographicSearchBox::SearchTitle] = new Athenaeum::TextFilter(QString(), Athenaeum::AbstractBibliographicCollection::TitleRole - Qt::UserRole, Qt::DisplayRole, this);
            standardFilters[Athenaeum::BibliographicSearchBox::SearchAuthors] = new Athenaeum::TextFilter(QString(), Athenaeum::AbstractBibliographicCollection::AuthorsRole - Qt::UserRole, Qt::DisplayRole, this);
            standardFilters[Athenaeum::BibliographicSearchBox::SearchAbstract] = new Athenaeum::TextFilter(QString(), Athenaeum::AbstractBibliographicCollection::AbstractRole - Qt::UserRole, Qt::DisplayRole, this);
            Athenaeum::ORFilter * orFilter = new Athenaeum::ORFilter(standardFilters.values(), this);
            standardFilters[Athenaeum::BibliographicSearchBox::SearchAll] = orFilter;

            libraryModel = new Athenaeum::LibraryModel(this);

            // Load libraries from disk
            QDir dataRoot(Utopia::profile_path());
            QList< Athenaeum::AbstractBibliographicModel * > models;
            QList< Athenaeum::RemoteQueryBibliographicModel * > searches;
            if (dataRoot.cd("library") || (dataRoot.mkdir("library") && dataRoot.cd("library"))) {
                QDir libraryRoot(dataRoot);
                if (libraryRoot.cd("collections") || (libraryRoot.mkdir("collections") && libraryRoot.cd("collections"))) {
                    foreach (const QFileInfo & library, libraryRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                        models << new Athenaeum::PersistentBibliographicModel(library.absoluteFilePath(), this);
                    }
                    if (models.isEmpty()) {
                        // Create a new model if none exist
                        QString uuid(QUuid::createUuid().toString());
                        QDir dir = libraryRoot.filePath(uuid.mid(1, uuid.size()-2));
                        Athenaeum::PersistentBibliographicModel * model = new Athenaeum::PersistentBibliographicModel(dir, this);
                        model->setTitle("My Library");
                        models << model;
                    }
                    filterProxyModel->setSourceModel(models.at(0));
                } else {
                    qDebug() << "=== Could not open library directory";
                }

                /*
                QDir searchesRoot(dataRoot);
                if (searchesRoot.cd("searches") || (searchesRoot.mkdir("searches") && searchesRoot.cd("searches"))) {
                    foreach (const QFileInfo & library, searchesRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                        searches << new RemoteQueryBibliographicModel(library.absoluteFilePath(), this);
                    }
                } else {
                    qDebug() << "=== Could not open searches directory";
                }
                */
            } else {
                qDebug() << "=== Could not open data directory";
            }
            //qDebug() << "=== Adding collections to library";
            foreach (Athenaeum::AbstractBibliographicModel * model, models) {
                libraryModel->appendModel(model);
            }
            /*
            foreach (RemoteQueryBibliographicModel * search, searches) {
                d->libraryModel->appendSearch(search);
            }
            */
            //qDebug() << "=== Collections added";

            /*
            // Select the first selectable item in the library view
            for (int i = 0; i < libraryModel->rowCount(); ++i) {
                QModelIndex index = libraryModel->index(i, 0, libraryModel->index(0, 0, QModelIndex()));
                QModelIndex indexTo = libraryModel->index(i, libraryModel->columnCount(), libraryModel->index(0, 0, QModelIndex()));
                if (index.flags() & Qt::ItemIsSelectable) {
                    libraryView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
                    libraryView->selectionModel()->select(QItemSelection(index, indexTo), QItemSelectionModel::Select);
                    break;
                }
            }
            */
        }

        searchLayout->addWidget(remoteSearchHeaderFrame, 0);
        remoteSearchLabelFrame->hide();
        searchLayout->addWidget(articleResultsView, 1);

        /////////////////////////////////////////////////////////////////////////////////
        // SearchControl layer

        QWidget * searchControlLayer = layers[SearchControlLayer] = new QWidget(mainWidget);
        searchControlLayer->setObjectName("search_control_layer");
        layerAnimationGroup.addAnimation(layerAnimations[SearchControlLayer] = new QPropertyAnimation(searchControlLayer, "geometry", this));
        layerAnimations[SearchControlLayer]->setEasingCurve(QEasingCurve::InOutSine);
        searchControlLayer->setFixedHeight(48);
        {
            QHBoxLayout * layout = new QHBoxLayout(searchControlLayer);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);

            QFrame * frame = new QFrame;
            layout->addWidget(frame);
        }

        /////////////////////////////////////////////////////////////////////////////////
        // Document layer

        QWidget * documentLayer = layers[DocumentLayer] = new QWidget(mainWidget);
        layerAnimationGroup.addAnimation(layerAnimations[DocumentLayer] = new QPropertyAnimation(documentLayer, "geometry", this));
        layerAnimations[DocumentLayer]->setEasingCurve(QEasingCurve::InOutSine);

        // In order from left to right: sliver, tab
        QHBoxLayout * mainLayout = new QHBoxLayout(documentLayer);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QFrame * tabBarWidget = new QFrame;
        tabBarWidget->setObjectName("tab_bar_frame");
        tabBarWidget->setFixedWidth(48);
        QVBoxLayout * tabBarWidgetLayout = new QVBoxLayout(tabBarWidget);
        tabBarWidgetLayout->setSpacing(0);
        tabBarWidgetLayout->setContentsMargins(0, 0, 0, 0);
        tabBar = new TabBar;
        tabBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        connect(tabBar, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTabChanged(int)));
        connect(tabBar, SIGNAL(layoutChanged()), this, SLOT(onTabLayoutChanged()));
        connect(tabBar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onTabBarCustomContextMenuRequested(const QPoint &)));
        connect(tabBar, SIGNAL(closeRequested(int)), this, SLOT(closeTab(int)));
        tabBarWidgetLayout->addWidget(tabBar, 0, Qt::AlignRight | Qt::AlignTop);
        mainLayout->addWidget(tabBarWidget);

        /////////////////////////////////////////////////////////////////////////////////
        // Tool layer

        // Sliver
        QWidget * sliverLayer = layers[SliverLayer] = new QWidget(mainWidget);
        sliverLayer->setObjectName("sliver_frame");
        layerAnimationGroup.addAnimation(layerAnimations[SliverLayer] = new QPropertyAnimation(sliverLayer, "geometry", this));
        layerAnimations[SliverLayer]->setEasingCurve(QEasingCurve::InOutSine);
        QVBoxLayout * sliverLayout = new QVBoxLayout(sliverLayer);
        sliver = new Sliver;
#ifndef UTOPIA_BUILD_DEBUG
        sliver->setFixedHeight(0);
        sliverLayer->hide();
#endif
        sliverLayout->setSpacing(0);
        sliverLayout->setContentsMargins(0, 0, 0, 0);
        sliverLayout->addStretch(1);
        sliverLayout->addWidget(sliver, 0);

        QButtonGroup * modeButtonGroup = new QButtonGroup(this);
        connect(modeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onModeChange(int)));

        selectingModeButton = new Utopia::HoldableButton;
        selectingModeButton->setIcon(generateToolIcon(AbstractProcessor::generateFromMonoPixmap(QPixmap(":/processors/selecting/icon.png"))));
        selectingModeButton->setText("Select");
        selectingModeButton->setToolTip("Select");
        selectingModeButton->setCheckable(true);
        selectingModeButton->setChecked(true);
        modeButtonGroup->addButton(selectingModeButton, DocumentView::SelectingMode);
#ifdef UTOPIA_BUILD_DEBUG
        sliver->layout()->addWidget(selectingModeButton);
        sliver->layout()->setAlignment(selectingModeButton, Qt::AlignHCenter);
#else
        selectingModeButton->hide();
#endif
        //connect(selectingModeButton, SIGNAL(clicked()), this, SLOT(onModeChangeSelecting()));

        highlightingModeButton = new Utopia::HoldableButton;
        highlightingModeButton->setIcon(generateToolIcon(AbstractProcessor::generateFromMonoPixmap(QPixmap(":/processors/highlighting/icon.png"))));
        highlightingModeButton->setText("Highlight");
        highlightingModeButton->setToolTip("Highlight");
        highlightingModeButton->setCheckable(true);
        modeButtonGroup->addButton(highlightingModeButton, DocumentView::HighlightingMode);
#ifdef UTOPIA_BUILD_DEBUG
        sliver->layout()->addWidget(highlightingModeButton);
        sliver->layout()->setAlignment(highlightingModeButton, Qt::AlignHCenter);
#else
        highlightingModeButton->hide();
#endif
        //connect(highlightingModeButton, SIGNAL(clicked()), this, SLOT(onModeChangeHighlighting()));
        connect(highlightingModeButton, SIGNAL(held()), this, SLOT(onHighlightingModeOptionsRequested()));

        {
            highlightingModeOptionFrame = new QFrame(0, Qt::Popup);
            highlightingModeOptionFrame->setObjectName("highlightingModeOptionFrame");
            QHBoxLayout * highlightingModeOptionFrameLayout = new QHBoxLayout(highlightingModeOptionFrame);
            highlightingModeOptionFrameLayout->setSpacing(0);
            highlightingModeOptionFrameLayout->setContentsMargins(4, 4, 4, 4);
            QButtonGroup * highlightingModeOptionButtonGroup = new QButtonGroup;
            typedef QPair< QString, QColor > _ColorPair;
            QList< _ColorPair > colors;
            colors << _ColorPair("Yellow", Qt::yellow);
            colors << _ColorPair("Green", Qt::green);
            colors << _ColorPair("Blue", Qt::cyan);
            colors << _ColorPair("Pink", Qt::magenta);
            bool first = true;
            foreach (const _ColorPair & color, colors) {
                static const QSize size(28, 28);
                QToolButton * button = new QToolButton;
                button->setCheckable(true);
                button->setText(color.first);
                button->setToolTip(color.first);
                button->setProperty("color", color.second);
                QPixmap block(size);
                block.fill(Qt::transparent);
                {
                    static const int pad = 4;
                    QPainter p(&block);
                    p.setBrush(color.second);
                    p.setPen(Qt::black);
                    p.drawRect(block.rect().adjusted(pad, pad, -pad-1, -pad-1));
                }
                button->setIcon(generateToolIcon(block, size));
                button->setFixedSize(size);
                if (first) {
                    first = false;
                    button->setChecked(true);
                }
                connect(button, SIGNAL(clicked()), this, SLOT(onHighlightingModeOptionChosen()));
                connect(button, SIGNAL(clicked()), highlightingModeOptionFrame, SLOT(hide()));
                highlightingModeOptionButtonGroup->addButton(button);
                highlightingModeOptionFrameLayout->addWidget(button);
            }
        }

#ifdef UTOPIA_BUILD_DEBUG
        doodlingModeButton = new Utopia::HoldableButton;
        doodlingModeButton->setIcon(QPixmap(":/processors/doodling/icon.png"));
        doodlingModeButton->setText("Doodle");
        sliver->layout()->addWidget(doodlingModeButton);
        sliver->layout()->setAlignment(doodlingModeButton, Qt::AlignHCenter);
        connect(doodlingModeButton, SIGNAL(pressed()), this, SLOT(onModeChangeDoodling()));
#endif

        tabLayout = new QStackedLayout;
        mainLayout->addLayout(tabLayout);




        /////////////////////////////////////////////////////////////////////////////////
        // Corner frame

        {
            cornerFrame = new QFrame(window());
            cornerFrame->setObjectName("corner_frame");
            cornerFrame->setFixedSize(48, 48);
            QHBoxLayout * layout2 = new QHBoxLayout(cornerFrame);
            layout2->setSpacing(0);
            layout2->setContentsMargins(0, 0, 0, 0);
            cornerButton = new QToolButton;
            QIcon icon(QPixmap(":/icons/article-search.png"));
            icon.addPixmap(QPixmap(":/icons/article-search-close.png"), QIcon::Normal, QIcon::On);
            cornerButton->setIcon(icon);
            cornerButton->setCheckable(true);
            cornerButton->setToolTip("Toggle article search");
            connect(cornerButton, SIGNAL(clicked(bool)), this, SLOT(onCornerButtonClicked(bool)));
            layout2->addWidget(cornerButton);
            connect(searchBox, SIGNAL(cancelRequested()), this, SLOT(showDocuments()));
        }




        {
            actionNextTab = new QAction("Switch to next tab", this);
            QList< QKeySequence > shortcuts;
            shortcuts << QKeySequence("Meta+Tab");
            shortcuts << QKeySequence::NextChild;
            actionNextTab->setShortcuts(shortcuts);
            QObject::connect(actionNextTab, SIGNAL(triggered()), tabBar, SLOT(nextTab()));
            window()->addAction(actionNextTab);
        }
        {
            actionPreviousTab = new QAction("Switch to previous tab", this);
            QList< QKeySequence > shortcuts;
            shortcuts << QKeySequence("Meta+Shift+Tab");
            shortcuts << QKeySequence::PreviousChild;
            actionPreviousTab->setShortcuts(shortcuts);
            QObject::connect(actionPreviousTab, SIGNAL(triggered()), tabBar, SLOT(previousTab()));
            window()->addAction(actionPreviousTab);
        }

        window()->resize(1000, 800);

        // Make status bar
        //setStatusBar(statusBar = new QStatusBar());

        // Create actions
        actionOpen = new QAction(QIcon(":/icons/open.png"), "Open...", this);
        actionOpen->setShortcut(QKeySequence::Open);
        QObject::connect(actionOpen, SIGNAL(triggered()), window(), SLOT(openFile()));
        window()->addAction(actionOpen);

        actionOpenUrl = new QAction("Open URL...", this);
        actionOpenUrl->setShortcut(QKeySequence("Shift+" + QKeySequence(QKeySequence::Open).toString()));
        QObject::connect(actionOpenUrl, SIGNAL(triggered()), window(), SLOT(openUrl()));
        window()->addAction(actionOpenUrl);

        actionOpenFromClipboard = new QAction("Open URL from clipboard", this);
        actionOpenFromClipboard->setShortcut(QKeySequence("Shift+" + QKeySequence(QKeySequence::Paste).toString()));
        actionOpenFromClipboard->setEnabled(false);
        QObject::connect(actionOpenFromClipboard, SIGNAL(triggered()), window(), SLOT(openFileFromClipboard()));
        window()->addAction(actionOpenFromClipboard);

        actionSave = new QAction(QIcon(":/icons/save.png"), "Save Copy...", this);
        actionSave->setShortcut(QKeySequence::Save);
        actionSave->setEnabled(false);
        QObject::connect(actionSave, SIGNAL(triggered()), window(), SLOT(saveFile()));
        window()->addAction(actionSave);

        actionClose = new QAction("Close", this);
        actionClose->setShortcut(QKeySequence::Close);
        QObject::connect(actionClose, SIGNAL(triggered()), this, SLOT(onClose()));
        window()->addAction(actionClose);

        // The following renaming is a temporary fix for commercial users who wish to
        // turn off the printing function
        bool printingEnabled = (defaults.value("printing").toString() != "disabled");

        actionPrint = new QAction("Print...", this);
        actionPrint->setShortcut(QKeySequence::Print);
        actionPrint->setEnabled(false);
        if (printingEnabled) {
            QObject::connect(actionPrint, SIGNAL(triggered()), this, SLOT(onPrint()));
        }
        window()->addAction(actionPrint);

        actionQuit = new QAction("Quit", this);
        actionQuit->setShortcut(QKeySequence::Quit);
        QObject::connect(actionQuit, SIGNAL(triggered()), QApplication::instance(), SLOT(closeAllWindows()));
        window()->addAction(actionQuit);

        actionCopy = new QAction("Copy", this);
        actionCopy->setShortcut(QKeySequence::Copy);
        QObject::connect(actionCopy, SIGNAL(triggered()), this, SLOT(copySelectedText()));
        window()->addAction(actionCopy);

        {
            QActionGroup * group = new QActionGroup(this);

            actionShowDocuments = new QAction("Show Documents", this);
            actionShowDocuments->setCheckable(true);
            actionShowDocuments->setChecked(true);
            QObject::connect(actionShowDocuments, SIGNAL(triggered()), this, SLOT(showDocuments()));
            window()->addAction(actionShowDocuments);
            group->addAction(actionShowDocuments);

#ifdef UTOPIA_BUILD_DEBUG
            actionShowLibrary = new QAction("Show Library", this);
            actionShowLibrary->setEnabled(false);
            actionShowLibrary->setCheckable(true);
            QObject::connect(actionShowLibrary, SIGNAL(triggered()), this, SLOT(showLibrary()));
            window()->addAction(actionShowLibrary);
            group->addAction(actionShowLibrary);
#endif

            actionShowSearch = new QAction("Search For Documents", this);
            actionShowSearch->setCheckable(true);
            QObject::connect(actionShowSearch, SIGNAL(triggered()), this, SLOT(showSearch()));
            window()->addAction(actionShowSearch);
            group->addAction(actionShowSearch);
        }

        actionShowHelp = new QAction("View Quick Start Guide", this);
        QObject::connect(actionShowHelp, SIGNAL(triggered()), window(), SLOT(showHelp()));
        window()->addAction(actionShowHelp);

        actionShowAbout = new QAction("About Utopia Documents", this);
        QObject::connect(actionShowAbout, SIGNAL(triggered()), window(), SLOT(showAbout()));
        window()->addAction(actionShowAbout);

        // Create other menu items
        window()->menuBar()->addMenu(menuFile = new QMenu("&File", window()));
        window()->menuBar()->addMenu(menuEdit = new QMenu("&Edit", window()));
        window()->menuBar()->addMenu(menuView = new QMenu("&View", window()));
        window()->menuBar()->addMenu(uiManager->menuWindow());
        window()->menuBar()->addMenu(uiManager->menuHelp());

        // Populate File menu
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionOpenUrl);
        menuFile->addAction(actionOpenFromClipboard);
        menuFile->addMenu(uiManager->menuRecent());
        menuFile->addSeparator();
        menuFile->addAction(actionSave);
        if (printingEnabled) {
            menuFile->addSeparator();
            menuFile->addAction(actionPrint);
        }
        menuFile->addSeparator();
        menuFile->addAction(actionClose);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);

        // Registering annotation / selection processors
        //foreach (AnnotationProcessor * processor, Utopia::instantiateAllExtensions< AnnotationProcessor >()) {
        //    d->annotationProcessors << boost::shared_ptr< AnnotationProcessor >(processor);
        //}
        foreach (SelectionProcessor * processor, Utopia::instantiateAllExtensions< SelectionProcessor >()) {
            //selectionProcessorActions << new SelectionProcessorAction(window(), processor);
        }
        connect(&primaryToolSignalMapper, SIGNAL(mapped(int)),
                this, SLOT(onPrimaryToolButtonClicked(int)));

        // Put some tools in the sliver
        activePrimaryToolAction = 0;
        primaryToolButtonGroup = new QButtonGroup(this);
        if (!selectionProcessorActions.isEmpty()) {
            static_cast< QBoxLayout * >(sliver->layout())->addStretch(1);
        }
        //addPrimaryToolButton(AbstractProcessor::generateFromMonoPixmap(QPixmap(":/processors/selecting/icon.png")), QString("Select"), -1)->click();
        for (int idx = 0; idx < selectionProcessorActions.size(); ++idx) {
            SelectionProcessorAction * processorAction = selectionProcessorActions.at(idx);
            addPrimaryToolButton(processorAction->icon(), processorAction->text(), idx);
        }
        //addPrimaryToolButton(QPixmap(":/icons/article-search.png"), QString("Search for article"), -2, 0);

        // Build menus
        rebuildMenus();

        // Set Mac HIToolbar
        window()->setUnifiedTitleAndToolBarOnMac(true);
        // Set up widget modes
        window()->setAttribute(Qt::WA_DeleteOnClose, true);
        window()->setAcceptDrops(true);

        // Layers
        toLayerState = currentLayerState = DocumentState;
        QMutableMapIterator< PapyroWindowPrivate::Layer, QPropertyAnimation * > iter(layerAnimations);
        while (iter.hasNext()) {
            iter.next();
            iter.value()->setStartValue(layerGeometryForState(iter.key(), toLayerState));
            iter.value()->setEndValue(iter.value()->startValue());
            iter.value()->setDuration(layerAnimtationDuration);
            iter.value()->targetObject()->setProperty(iter.value()->propertyName(), iter.value()->startValue());
        }

        updateHighlightingModeButton();
    }

    QRect PapyroWindowPrivate::layerGeometry(Layer layer) const
    {
        if (QWidget * layerWidget = layers.value(layer, 0)) {
            return layerWidget->geometry();
        } else {
            return QRect();
        }
    }

    QRect PapyroWindowPrivate::layerGeometryForState(Layer layer, LayerState layerState) const
    {
        if (QWidget * layerWidget = layers.value(layer, 0)) {
            QRect bounds(layerWidget->parentWidget()->rect());
            switch (layer) {
            case SearchControlLayer: {
                int searchControlHeight = layers[SearchControlLayer]->height();
                return QRect(0, bounds.height() - searchControlHeight, bounds.width(), searchControlHeight);
            }
            case SliverLayer: {
                int searchControlHeight = layers[SearchControlLayer]->height();
                QRect rect(0, bounds.height() - sliver->sizeHint().height() - searchControlHeight, sliver->width(), sliver->sizeHint().height());
                switch (layerState) {
                case DocumentState:
                    return rect;
                case SearchState:
                default:
                    return rect.translated(-sliver->width(), 0);
                }
            }
            case SearchLayer: {
                int searchControlHeight = layers[SearchControlLayer]->height();
                bounds.setWidth(300);
                bounds.setBottom(bounds.height() - searchControlHeight);
                switch (layerState) {
                case SearchState:
                    return bounds;
                case DocumentState:
                default:
                    return bounds.translated(sliver->width(), 0);
                }
            }
            case DocumentLayer:
                switch (layerState) {
                case SearchState:
                    return bounds.translated(300, 0);
                case DocumentState:
                default:
                    return bounds;
                }
            default:
                return bounds;
            }
        }
        return QRect();
    }

    void PapyroWindowPrivate::moveTabToNewWindow(int index)
    {
        if (PapyroTab * tab = takeTab(index)) {
            PapyroWindow * newWindow = new PapyroWindow(tab, 0, 0);
            newWindow->show();
        }
    }

    PapyroTab * PapyroWindowPrivate::newTab()
    {
        PapyroTab * tab = new PapyroTab;
        tab->documentView()->setInteractionMode(interactionMode);
        tab->documentView()->setHighlightColor(highlightingColor);
        addTab(tab);

        // Add to window menu
        QAction * raiseAction = new RaiseTabAction(tab, tab);
        raiseAction->setActionGroup(uiManager->menuWindowActionGroup());
        uiManager->menuWindowMaster()->addAction(raiseAction);

        return tab;
    }

    void PapyroWindowPrivate::onResolverRunnableCompleted(QModelIndex index, QVariantMap metadata)
    {
        QUrl pdfUrl = metadata.value("pdf").toUrl();
        QUrl url = metadata.value("url").toUrl();
        bool raise = metadata.value("_raise").toBool();

        articleResultsView->model()->setData(index, Athenaeum::AbstractBibliographicCollection::IdleItemState, Athenaeum::AbstractBibliographicCollection::ItemStateRole);
        articleResultsView->setIndexWidget(index, 0);

        if (pdfUrl.isValid()) {
            // Now we have a valid URL, we must do our best to resolve it to a PDF
            // file, then open it.
            articleResultsView->model()->setData(index, pdfUrl, Athenaeum::AbstractBibliographicCollection::PdfRole);
            window()->open(pdfUrl, raise);
        } else if (url.isValid()) {
            QDesktopServices::openUrl(url);
        } else {
            // FIXME attempt to use the local discoverer
        }
    }

    void PapyroWindowPrivate::onArticleActivated(const QModelIndex & index)
    {
        closeArticlePreview();

        // Only bother trying to launch an idle article
        Athenaeum::AbstractBibliographicCollection::ItemState state = index.data(Athenaeum::AbstractBibliographicCollection::ItemStateRole).value< Athenaeum::AbstractBibliographicCollection::ItemState >();
        if (state == Athenaeum::AbstractBibliographicCollection::IdleItemState) {
            // It is the job of this method to do something useful and expected when a user
            // activates (double-clicks) an article.
            bool raise = ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0);

            // If the record includes a local filename, launch that PDF file
            QUrl pdfUrl(index.data(Athenaeum::AbstractBibliographicCollection::PdfRole).toUrl());
            QUrl url(index.data(Athenaeum::AbstractBibliographicCollection::UrlRole).toUrl());
            if (pdfUrl.isValid()) {
                window()->open(pdfUrl, raise);
            } else {
                // If no local file is found, we must attempt to generate or find a URL with
                // which to search for the article. Sometimes such a URL is already provided
                // in the citation record. If not, we must fall back to some intelligent
                // guesswork
                if (Athenaeum::BibliographicItem * item = index.data(Athenaeum::AbstractBibliographicCollection::ItemRole).value< Athenaeum::BibliographicItem * >()) {
                    QVariantMap metadata = item->toMap();
                    Athenaeum::ResolverRunnable * resolverRunnable = new Athenaeum::ResolverRunnable(index, metadata);

                    QWidget * spinnerWidget = new QWidget;
                    QHBoxLayout * spinnerLayout = new QHBoxLayout(spinnerWidget);
                    spinnerLayout->setContentsMargins(4, 4, 4, 4);
                    Utopia::Spinner * spinner = new Utopia::Spinner;
                    spinner->setColor(Qt::white);
                    spinner->setFixedSize(34, 48);
                    spinnerLayout->addWidget(spinner, 0, Qt::AlignLeft | Qt::AlignVCenter);

                    connect(resolverRunnable, SIGNAL(started()), spinner, SLOT(start()));
                    connect(resolverRunnable, SIGNAL(completed()), spinner, SLOT(stop()));
                    connect(resolverRunnable, SIGNAL(completed(QModelIndex, QVariantMap)), this, SLOT(onResolverRunnableCompleted(QModelIndex, QVariantMap)));
                    articleResultsView->setIndexWidget(index, spinnerWidget);

                    articleResultsView->model()->setData(index, QVariant::fromValue(Athenaeum::AbstractBibliographicCollection::BusyItemState), Athenaeum::AbstractBibliographicCollection::ItemStateRole);

                    QThreadPool::globalInstance()->start(resolverRunnable);
                }
            }
        }
    }

    void PapyroWindowPrivate::onArticlePreviewRequested(const QModelIndex & index)
    {
        static const QString css = "h1 { color: #000; } "
                                   "p { color: #222; } "
                                   "p.authors, p .publicationTitle, p .publisher { font-style: italic; } ";

        typedef QPair< int, QString > _PAIR;
        static QList< _PAIR > tpls;
        if (tpls.isEmpty()) {
            tpls << _PAIR(Athenaeum::AbstractBibliographicCollection::TitleRole, "<h1 style='font-size: large'>%1</h1>");
            tpls << _PAIR(Athenaeum::AbstractBibliographicCollection::SubtitleRole, "<h2 style='font-size: medium'>%1</h2>");
            tpls << _PAIR(Athenaeum::AbstractBibliographicCollection::AuthorsRole, "<p align='justify' class='authors'>%1.</p>");
            tpls << _PAIR(Athenaeum::AbstractBibliographicCollection::YearRole, "<p align='justify' class='publication'>%1</p>");
            tpls << _PAIR(Athenaeum::AbstractBibliographicCollection::AbstractRole, "<p align='justify' class='abstract'>%1</p>");
        }

        bool needNewPreview = articlePreviewIndex != index;
        //qDebug() << needNewPreview << articlePreviewIndex << index;

        if (index.isValid()) {
            QRect itemRect(articleResultsView->visualRect(index));
            QPoint itemRightMiddle(itemRect.right(), (itemRect.top() + itemRect.bottom()) / 2);
            if (needNewPreview) {
                closeArticlePreview();
                articlePreviewBubble = new Utopia::BubbleWidget(0, Qt::Tool);
                articlePreviewBubble.data()->setObjectName("article_preview_bubble");
                articlePreviewBubble.data()->setAttribute(Qt::WA_DeleteOnClose, true);
                articlePreviewBubble.data()->setFixedSize(600, 250);
                articlePreviewBubble.data()->installEventFilter(this);
                QVBoxLayout * layout = new QVBoxLayout(articlePreviewBubble.data());

                QString html;
                foreach (const _PAIR & pair, tpls) {
                    switch (pair.first) {
                    case Athenaeum::AbstractBibliographicCollection::AuthorsRole: {
                        QStringList names;
                        foreach (const QString & author, index.data(Athenaeum::AbstractBibliographicCollection::AuthorsRole).toStringList()) {
                            names << (author.section(", ", 1) + " " + author.section(", ", 0, 0));
                        }
                        if (!names.isEmpty()) {
                            html += pair.second.arg(names.join(", "));
                        }
                        break;
                    }
                    case Athenaeum::AbstractBibliographicCollection::YearRole: {
                        QStringList publication;

                        QString publicationTitle = index.data(Athenaeum::AbstractBibliographicCollection::PublicationTitleRole).toString();
                        if (!publicationTitle.isEmpty()) {
                            publication << "<span class='publicationTitle'>\"" + publicationTitle + "\"</span>";
                        }

                        QString year = index.data(Athenaeum::AbstractBibliographicCollection::YearRole).toString();
                        if (!year.isEmpty()) {
                            publication << "<span class='year'>(" + year + ")</span>";
                        }

                        QString volume = index.data(Athenaeum::AbstractBibliographicCollection::VolumeRole).toString();
                        QString issue = index.data(Athenaeum::AbstractBibliographicCollection::IssueRole).toString();
                        QString volumeIssue;
                        if (!volume.isEmpty()) {
                            volumeIssue += "<span class='volume'>" + volume + "</span>";
                        }
                        if (!issue.isEmpty()) {
                            if (volumeIssue.isEmpty()) {
                                volumeIssue += ":";
                            }
                            volumeIssue += "<span class='issue'>" + issue + "</span>";
                        }
                        if (!volumeIssue.isEmpty()) {
                            publication << volumeIssue;
                        }

                        QString pogeFrom = index.data(Athenaeum::AbstractBibliographicCollection::PageFromRole).toString();
                        QString pageTo = index.data(Athenaeum::AbstractBibliographicCollection::PageToRole).toString();
                        QString pages;
                        if (!pogeFrom.isEmpty()) {
                            if (!pageTo.isEmpty()) {
                                pages += "p";
                            }
                            pages += "p<span class='pogeFrom'>" + pogeFrom + "</span>";
                        }
                        if (!pageTo.isEmpty()) {
                            if (!pages.isEmpty()) {
                                pages += "-";
                            } else {
                                pages += "p";
                            }
                            pages += "<span class='pageTo'>" + pageTo + "</span>";
                        }
                        if (!pages.isEmpty()) {
                            publication << pages;
                        }

                        QString publisher = index.data(Athenaeum::AbstractBibliographicCollection::PublisherRole).toString();
                        if (!publisher.isEmpty()) {
                            publication << "<span class='publisher'>" + publisher + "</span>";
                        }

                        if (!publication.isEmpty()) {
                            html += pair.second.arg(publication.join(" "));
                        }
                        break;
                    }
                    default: {
                        QString value = index.data(pair.first).toString();
                        if (!value.isEmpty()) {
                            html += pair.second.arg(value);
                        }
                        break;
                    }
                    }
                }

                QTextDocument * document = new QTextDocument;
                document->addResource(QTextDocument::StyleSheetResource, QUrl("style.css"), css);
                document->setHtml("<html><head><link rel='stylesheet' type='text/css' href='style.css'></head><body>" + html + "</body></html>");

                QTextEdit * textEdit = new QTextEdit;
                textEdit->setDocument(document);
                textEdit->setReadOnly(true);

                layout->addWidget(textEdit);
                layout->setContentsMargins(0, 0, 0, 0);
            }

            if (articlePreviewBubble) {
                articlePreviewBubble.data()->show(articleResultsView->mapToGlobal(itemRightMiddle));
            }
        } else {
            closeArticlePreview();
        }

        articlePreviewIndex = index;
    }

    void PapyroWindowPrivate::onArticleViewCustomContextMenuRequested(const QPoint & pos)
    {
        // For use in plurals
        QString s;

        // Get the current selection
        int count = articleResultsView->selectionModel()->selectedIndexes().size();

        if (count > 0) {
            // New menu
            QMenu menu;

            // Get names right
            if (count > 1) {
                s = "s";
            }

            menu.addAction("Open", this, SLOT(openSelectedArticles()));
            //menu.addAction("Delete", this, SLOT(deleteSelectedArticles()));
            menu.addSeparator();
            if (!exporters.isEmpty()) {
                menu.addAction("Export Selected Citation"+s+"...", this, SLOT(exportCitationsOfSelectedArticles()));
            }

            menu.exec(articleResultsView->viewport()->mapToGlobal(pos));
        }
    }

    void PapyroWindowPrivate::onClipboardDataChanged()
    {
        const QMimeData * mimeData = QApplication::clipboard()->mimeData();
        actionOpenFromClipboard->setEnabled(!checkForSupportedUrls(mimeData).isEmpty());
    }

    void PapyroWindowPrivate::onClose()
    {
        closeTab(tabBar->currentIndex());
    }

    void PapyroWindowPrivate::onCornerButtonClicked(bool checked)
    {
        if (QToolButton * button = qobject_cast< QToolButton * >(sender())) {
            changeToLayerState(checked ? SearchState : DocumentState);
        }
    }

    void PapyroWindowPrivate::onCurrentTabChanged(int index)
    {
        if (index >= 0 && index < tabLayout->count()) {
            // Get the current tool action
            if (PapyroTab * oldTab = currentTab()){
                oldTab->setActiveSelectionProcessorAction(activePrimaryToolAction);
                // Make sure exposure is turned off for soon-to-be disappearing tabs
                if (DocumentView * documentView = oldTab->documentView()) {
                    documentView->setExposing(false);
                }
            }

            // Raise tab target
            tabLayout->setCurrentIndex(index);

            // Set the current tool action
            if (PapyroTab * tab = currentTab()) {
                tab->setActiveSelectionProcessorAction(activePrimaryToolAction);
            }

            // Rebuild menus
            rebuildMenus();
            // Update tab info
            updateTabInfo();
            // Make sure UI matches the document
            onTabDocumentChanged();
            // Make sure we're out of search mode
            if (toLayerState == PapyroWindowPrivate::SearchState) {
                changeToLayerState(PapyroWindowPrivate::DocumentState);
            }

            emit currentTabChanged();
        }
    }

    void PapyroWindowPrivate::onFilterRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain searchDomain)
    {
        // Apply filter to article view
        if (text.isEmpty()) {
            filterProxyModel->setFilter(0);
        } else {
            foreach (Athenaeum::AbstractFilter * filter, standardFilters.values()) {
                if (Athenaeum::TextFilter * textFilter = qobject_cast< Athenaeum::TextFilter * >(filter)) {
                    textFilter->setFixedString(text);
                }
            }
            filterProxyModel->setFilter(standardFilters.value(searchDomain, 0));
        }
    }
    void PapyroWindowPrivate::onHighlightingModeOptionsRequested()
    {
        int top = highlightingModeButton->mapToGlobal(highlightingModeButton->rect().topRight()).y();
        int right = sliver->mapToGlobal(sliver->rect().topRight()).x();
        highlightingModeOptionFrame->move(right, top);
        highlightingModeOptionFrame->show();
    }

    void PapyroWindowPrivate::onHighlightingModeOptionChosen()
    {
        QColor color(sender()->property("color").value< QColor >());
        highlightingColor = color;
        foreach (PapyroTab * tab, tabs()) {
            tab->documentView()->setHighlightColor(highlightingColor);
        }
        highlightingModeButton->click();
        updateHighlightingModeButton();
    }

    void PapyroWindowPrivate::onModeChange(int mode_as_int)
    {
        DocumentView::InteractionMode mode = (DocumentView::InteractionMode) mode_as_int;
        PapyroTab * tab = currentTab();
        if (mode == DocumentView::HighlightingMode && tab && tab->document() &&
            (!tab->document()->textSelection().empty() ||
             !tab->document()->areaSelection().empty())) {

            tab->documentView()->highlightSelection();

            switch (interactionMode) {
            case DocumentView::SelectingMode:
                selectingModeButton->click();
                break;
            case DocumentView::HighlightingMode:
                highlightingModeButton->click();
                break;
            case DocumentView::DoodlingMode:
                doodlingModeButton->click();
                break;
            }
        } else {
            foreach (PapyroTab * tab, tabs()) {
                tab->documentView()->setInteractionMode(mode);
            }
            interactionMode = mode;
        }
    }

    void PapyroWindowPrivate::onModeChangeSelecting()
    {
        if (PapyroTab * tab = currentTab()) {
            tab->documentView()->setInteractionMode(DocumentView::SelectingMode);
        }
    }

    void PapyroWindowPrivate::onModeChangeDoodling()
    {
        if (PapyroTab * tab = currentTab()) {
            tab->documentView()->setInteractionMode(DocumentView::DoodlingMode);
        }
    }

    void PapyroWindowPrivate::onModeChangeHighlighting()
    {
        if (PapyroTab * tab = currentTab()) {
            tab->documentView()->setInteractionMode(DocumentView::HighlightingMode);
        }
    }

    void PapyroWindowPrivate::onNewWindow()
    {
        PapyroWindow * newWindow = new PapyroWindow;
        newWindow->show();
    }

    void PapyroWindowPrivate::onPrimaryToolButtonClicked(int idx)
    {
        if (PapyroTab * tab = currentTab()) {
            switch (idx) {
            case -1: // Reset to select
                activePrimaryToolAction = 0;
                break;
            case -2: // Show search layer
                actionShowSearch->trigger();
                return;
            default:
                activePrimaryToolAction = selectionProcessorActions.at(idx);
                // FIXME do something?
                break;
            }
        }
    }

    void PapyroWindowPrivate::onPrint()
    {
        if (PapyroTab * tab = currentTab()) {
            printer->print(tab->document(), window());
        }
    }

    void PapyroWindowPrivate::onRemoteSearchStateChanged(Athenaeum::AbstractBibliographicCollection::State /*state*/)
    {
        bool busy = false;
        // Check all the remote searches and decide upon a state
        foreach (Athenaeum::RemoteQueryBibliographicModel * remoteSearch, remoteSearches) {
            if (remoteSearch->state() == Athenaeum::AbstractBibliographicCollection::BusyState) {
                busy = true;
                break;
            }
        }

        if (busy) {
            remoteSearchLabelSpinner->start();
        } else {
            remoteSearchLabelSpinner->stop();
        }
    }

    void PapyroWindowPrivate::onSearchRequested(const QString & text, Athenaeum::BibliographicSearchBox::SearchDomain searchDomain)
    {
        // Only work on sensible input
        const QString term = text.trimmed();
        if (term.isEmpty()) {
            return;
        }

        // Create query map
        QVariantMap query;
        query["query"] = term;
        switch (searchDomain) {
        case Athenaeum::BibliographicSearchBox::SearchTitle:
            query["domain"] = "title";
            break;
        case Athenaeum::BibliographicSearchBox::SearchAuthors:
            query["domain"] = "authors";
            break;
        case Athenaeum::BibliographicSearchBox::SearchAbstract:
            query["domain"] = "abstract";
            break;
        case Athenaeum::BibliographicSearchBox::SearchAll:
        default:
            break;
        }

        // Start by removing the previous search
        closeArticlePreview();
        removeRemoteSearch();

        QDir searchesRoot(Utopia::profile_path());
        if (searchesRoot.cd("library") || (searchesRoot.mkdir("library") && searchesRoot.cd("library"))) {
            if (searchesRoot.cd("searches") || (searchesRoot.mkdir("searches") && searchesRoot.cd("searches"))) {
                foreach (const std::string & remoteQueryExtensionName, Utopia::registeredExtensionNames< Athenaeum::RemoteQuery >()) {
                    if (!remoteQueryExtensionName.empty()) {
                        // Create a new remote query if none exist
                        QString uuid(QUuid::createUuid().toString());
                        QDir dir = searchesRoot.filePath(uuid.mid(1, uuid.size()-2));
                        Athenaeum::RemoteQueryBibliographicModel * remoteQuery = new Athenaeum::RemoteQueryBibliographicModel(QString::fromStdString(remoteQueryExtensionName), dir, this);
                        connect(remoteQuery, SIGNAL(stateChanged(Athenaeum::AbstractBibliographicCollection::State)),
                                this, SLOT(onRemoteSearchStateChanged(Athenaeum::AbstractBibliographicCollection::State)));
                        aggregatingProxyModel->appendSourceModel(remoteQuery);
                        filterProxyModel->setSourceModel(aggregatingProxyModel);
                        remoteQuery->setQuery(query);
                        remoteQuery->setTitle("pubmed: " + term);
                        remoteSearches.append(remoteQuery);
                    }
                }
                static const QString searchTpl("<span>Search results for: <strong>%1</strong></span>");
                remoteSearchLabel->setText(searchTpl.arg(term));
                remoteSearchLabelFrame->show();
            }
        }

        // FIXME what about errors?
        // FIXME use searchDomain properly
    }

    void PapyroWindowPrivate::onTabBarCustomContextMenuRequested(const QPoint & pos)
    {
        int focus = tabBar->indexAt(pos);
        PapyroTab * tab = qobject_cast< PapyroTab * >(tabLayout->widget(focus));
        if (tab && focus >= 0) {
            QMenu menu;
            if (focus != tabBar->currentIndex()) {
                QSignalMapper * mapper = new QSignalMapper(&menu);
                QAction * action = menu.addAction("Raise Tab", mapper, SLOT(map()));
                mapper->setMapping(action, focus);
                connect(mapper, SIGNAL(mapped(int)), tabBar, SLOT(setCurrentIndex(int)));
            }
            if (tabBar->count() > 1) {
                QSignalMapper * mapper = new QSignalMapper(&menu);
                QAction * action = menu.addAction("Move to New Window", mapper, SLOT(map()));
                mapper->setMapping(action, focus);
                connect(mapper, SIGNAL(mapped(int)), this, SLOT(moveTabToNewWindow(int)));
            }
            menu.addSeparator();
            {
                QString text = tab->state() == PapyroTab::EmptyState && tabBar->count() == 1 ? "Close Window" : "Close Tab";
                QSignalMapper * mapper = new QSignalMapper(&menu);
                QAction * action = menu.addAction(text, mapper, SLOT(map()));
                mapper->setMapping(action, focus);
                connect(mapper, SIGNAL(mapped(int)), this, SLOT(closeTab(int)));
            }
            if (tabBar->count() > 1) {
                QSignalMapper * mapper = new QSignalMapper(&menu);
                QAction * action = menu.addAction("Close Other Tabs", mapper, SLOT(map()));
                mapper->setMapping(action, focus);
                connect(mapper, SIGNAL(mapped(int)), this, SLOT(closeOtherTabs(int)));
            }
            menu.exec(tabBar->mapToGlobal(pos));
        }
    }

    void PapyroWindowPrivate::onTabContextMenu(QMenu * menu)
    {
        qDebug() << "PapyroWindowPrivate::onTabContextMenu";
    }

    void PapyroWindowPrivate::onTabDocumentChanged()
    {
        if (PapyroTab * tab = currentTab()) {
            // Enable / disable menu items
            actionSave->setEnabled(!tab->isEmpty());
            actionPrint->setEnabled(!tab->isEmpty());
        } else {
            actionSave->setEnabled(false);
            actionPrint->setEnabled(false);
        }
        updateTabVisibility();
    }

    void PapyroWindowPrivate::onTabLayoutChanged()
    {
        bool multiple = tabBar->count() > 1;
        actionNextTab->setEnabled(multiple);
        actionPreviousTab->setEnabled(multiple);
        updateTabInfo();
        updateTabVisibility();
    }

    void PapyroWindowPrivate::onTabStateChanged(PapyroTab::State state)
    {
        // If no title has been found, set it to something sensible
        if (state == PapyroTab::IdleState) {
            if (PapyroTab * tab = qobject_cast< PapyroTab * >(sender())) {
                if (tab->title() == "Loading...") {
                    static const int preview = 80;
                    QString title = "Untitled";
                    QString text = qStringFromUnicode(tab->document()->substr(0, preview + 3)->text());
                    if (text.size() == (preview + 3)) {
                        text = text.mid(0, preview) + "...";
                    }
                    if (!text.isEmpty()) {
                        title += " - \"" + text + "\"";
                    }
                    tab->setTitle(title);
                }
            }
        }

        updateTabVisibility();
    }

    void PapyroWindowPrivate::onTabTitleChanged(const QString & /*title*/)
    {
        updateTabInfo();
    }

    void PapyroWindowPrivate::onTabUrlChanged(const QUrl & url)
    {
        if (url.isValid()) {
            uiManager->addRecentFile(url);
        }
        updateTabInfo();
    }

    void PapyroWindowPrivate::onUrlRequested(const QUrl & url, const QString & targetString)
    {
        static QRegExp paramSplitter("\\s*;\\s*");
        static QRegExp keyValueSplitter("\\s*=\\s*");
        QRegExp rectSplitter("\\[\\s*(\\d+(?:\\.\\d+)?)\\s+(\\d+(?:\\.\\d+)?)\\s+(\\d+(?:\\.\\d+)?)\\s+(\\d+(?:\\.\\d+)?)\\s*\\]");
        QRegExp pointSplitter("\\[\\s*(\\d+(?:\\.\\d+)?)\\s+(\\d+(?:\\.\\d+)?)\\s*\\]");
        QRegExp textSplitter("\\[([^\\]]+)\\]");

        QStringList sections(targetString.split(paramSplitter));
        QString target;
        QMap< QString, QString > params;
        QVariantMap showParams;
        if (!sections.isEmpty()) {
            target = sections.at(0);
            for (int i = 1; i < sections.size(); ++i) {
                QString section(sections.at(i));
                params[section.section(keyValueSplitter, 0, 0).toLower()] = section.section(keyValueSplitter, 1, 1);
            }
        }

        // Try to resolve the anchor point of the target
        if (params.contains("show")) {
            showParams["show"] = params.value("show");
        }
        if (params.contains("page")) {
            showParams["page"] = params.value("page").toInt();
        }
        if (pointSplitter.exactMatch(params.value("pos"))) {
            showParams["pos"] = QPointF(pointSplitter.cap(1).toFloat(),
                                        pointSplitter.cap(2).toFloat());
        } else if (rectSplitter.exactMatch(params.value("rect"))) {
            showParams["rect"] = QRectF(QPointF(rectSplitter.cap(1).toFloat(),
                                                rectSplitter.cap(2).toFloat()),
                                        QPointF(rectSplitter.cap(3).toFloat(),
                                                rectSplitter.cap(4).toFloat()));
        } else if (params.contains("anchor")) {
            showParams["anchor"] = params.value("anchor");
        } else if (textSplitter.exactMatch(params.value("text"))) {
            showParams["text"] = QUrl::fromPercentEncoding(textSplitter.cap(1).toAscii());
        }

        if (target == "tab" || target == "pdf") {
            if (!url.isRelative()) {
                bool raise = ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0);
                window()->open(url, raise, showParams);
            } else if (PapyroTab * tab = currentTab()) {
                tab->documentView()->showPage(showParams);
            }
        } else if (target == "window") {
            PapyroWindow * window = new PapyroWindow;
            window->open(url, true, showParams);
        } else {
            QDesktopServices::openUrl(url);
        }
    }

    void PapyroWindowPrivate::openSelectedArticles()
    {
        foreach (const QModelIndex & index, articleResultsView->selectionModel()->selectedIndexes()) {
            onArticleActivated(index);
        }
    }

    void PapyroWindowPrivate::rebuildMenus()
    {
        // Remove the old tab's actions from this window
        foreach (QPointer< QAction > action, currentTabActions) {
            if (action) {
                window()->removeAction(action);
            }
        }
        currentTabActions.clear();
        // And replace them with those of the new tab
        if (PapyroTab * tab = currentTab()) {
            foreach (QAction * action, tab->actions()) {
                currentTabActions << action;
                window()->addAction(action);
            }
        } else {
            // FIXME Library shortcut actions
        }

        // Populate Edit menu
        menuEdit->clear();
        menuEdit->addAction(actionCopy);
        menuEdit->addSeparator();
        if (PapyroTab * tab = currentTab()) {
            menuEdit->addAction(tab->action(PapyroTab::QuickSearch));
            menuEdit->addAction(tab->action(PapyroTab::QuickSearchNext));
            menuEdit->addAction(tab->action(PapyroTab::QuickSearchPrevious));
            menuEdit->addSeparator();
        } else {
            // FIXME Library menu items
        }
        menuEdit->addAction(uiManager->actionPreferences());

        // Populate View menu
        menuView->clear();
        if (PapyroTab * tab = currentTab()) {
            menuView->addMenu(tab->documentView()->layoutMenu());
            menuView->addMenu(tab->documentView()->zoomMenu());
            menuView->addSeparator();
            menuView->addAction(tab->action(PapyroTab::ToggleSidebar));
            menuView->addAction(tab->action(PapyroTab::ToggleLookupBar));
            menuView->addAction(tab->action(PapyroTab::TogglePager));
            menuView->addAction(tab->action(PapyroTab::ToggleImageBrowser));
            menuView->addSeparator();
        } else {
            // FIXME Library menu items
        }
        menuView->addSeparator();
        menuView->addAction(actionNextTab);
        menuView->addAction(actionPreviousTab);
        menuView->addSeparator();
        menuView->addAction(actionShowDocuments);
        menuView->addAction(actionShowSearch);
#ifdef UTOPIA_BUILD_DEBUG // Only needed once library is in place
        menuView->addAction(actionShowLibrary);
#endif
    }

    void PapyroWindowPrivate::removeRemoteSearch()
    {
        // Reset the filter's source model, back to the first collection
        filterProxyModel->setSourceModel(libraryModel->modelCount() > 0 ? libraryModel->modelAt(0) : 0);
        aggregatingProxyModel->clear();

        // Remove the bar from the UI
        remoteSearchLabelFrame->hide();

        // Delete the remoteQuery, purging it along the way
        foreach (Athenaeum::RemoteQueryBibliographicModel * remoteSearch, remoteSearches) {
            remoteSearch->purge();
        }
        remoteSearches.clear();
    }

    void PapyroWindowPrivate::showDocuments()
    {
        changeToLayerState(DocumentState);
    }

    void PapyroWindowPrivate::showLibrary()
    {
    }

    void PapyroWindowPrivate::showSearch()
    {
        changeToLayerState(SearchState);
    }

    PapyroTab * PapyroWindowPrivate::tabAt(int index)
    {
        return qobject_cast< PapyroTab * >(tabLayout->widget(index));
    }

    QList< PapyroTab * > PapyroWindowPrivate::tabs()
    {
        QList< PapyroTab * > tabs;
        int index = 0;
        while (PapyroTab * tab = tabAt(index++)) {
            tabs << tab;
        }
        return tabs;
    }

    PapyroTab * PapyroWindowPrivate::takeTab(int index)
    {
        // close the active tab
        if (PapyroTab * tab = tabAt(index)) {
            tab->disconnect(this);
            tabLayout->removeWidget(tab);
            tabBar->removeTab(index);
            return tab;
        }
        return 0;
    }

    void PapyroWindowPrivate::updateHighlightingModeButton()
    {
        QPixmap pixmap(":/processors/highlighting/icon.png");
        QPixmap overlay(":/processors/highlighting/icon-overlay.png");
        {
            QPainter p(&overlay);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.setPen(Qt::NoPen);
            p.setBrush(highlightingColor);
            p.drawRect(overlay.rect());
        }
        {
            QPainter p(&pixmap);
            p.drawPixmap(0, 0, overlay);
        }
        highlightingModeButton->setIcon(generateToolIcon(AbstractProcessor::generateFromMonoPixmap(pixmap)));
    }

    void PapyroWindowPrivate::updateManualLayouts()
    {
        if (layers.contains(SearchControlLayer) && layers[SearchControlLayer]) {
            tabBar->setFixedHeight(qMax(layers[SearchControlLayer]->geometry().top() - sliver->geometry().height(), 0));
        }
        cornerFrame->move(0, window()->height() - cornerFrame->height());
    }

    void PapyroWindowPrivate::updateTabInfo()
    {
        QString title;
        QString filepath;
        if (PapyroTab * tab = currentTab()) {
            QString tabTitle(tab->title());
            if (!tabTitle.isEmpty()) {
                title += " - " + tabTitle;
            }
            QUrl tabUrl(tab->url());
            if (tabUrl.isLocalFile()) {
                filepath = tabUrl.toLocalFile();
            }
        }
        window()->setWindowTitle("Utopia Documents" + title);
        window()->setWindowFilePath(filepath);
    }

    void PapyroWindowPrivate::updateTabVisibility()
    {
        PapyroTab * tab = currentTab();
        bool show = (tab && (tabLayout->count() > 1 || tab->state() != PapyroTab::EmptyState));
        tabBar->setVisible(show);
    }

    PapyroWindow * PapyroWindowPrivate::window()
    {
        return qobject_cast< PapyroWindow * >(Utopia::AbstractWindowPrivate::window());
    }

    const PapyroWindow * PapyroWindowPrivate::window() const
    {
        return qobject_cast< const PapyroWindow * >(Utopia::AbstractWindowPrivate::window());
    }


    /// PapyroRecentUrlHelper /////////////////////////////////////////////////////////////////////


    PapyroRecentUrlHelper::PapyroRecentUrlHelper()
        : QObject(0)
    {}

    void PapyroRecentUrlHelper::activateRecentUrl(QUrl url)
    {
        bool raised = false;
        PapyroWindow * active = 0;
        boost::shared_ptr< Utopia::UIManager > manager(Utopia::UIManager::instance());
        foreach (PapyroWindow * window, manager->windows< PapyroWindow >()) {
            if (!raised) {
                int index = window->indexOf(url);
                if (index >= 0) {
                    window->raiseTab(index);
                    window->show();
                    window->activateWindow();
                    window->raise();
                    raised = true;
                }
            }
            if (active == 0 || window->isActiveWindow()) {
                active = window;
            }
        }
        if (!raised) {
            if (!active) {
                active = new PapyroWindow;
            }
            active->open(url);
            active->show();
            active->activateWindow();
            active->raise();
        }
    }

    void PapyroRecentUrlHelper::closeAll()
    {
        PapyroWindow::closeAll();
    }

    boost::shared_ptr< PapyroRecentUrlHelper > PapyroRecentUrlHelper::instance()
    {
        static boost::weak_ptr< PapyroRecentUrlHelper > singleton;
        boost::shared_ptr< PapyroRecentUrlHelper > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< PapyroRecentUrlHelper >(new PapyroRecentUrlHelper());
            singleton = shared;
        }
        return shared;
    }


    /// PapyroWindow //////////////////////////////////////////////////////////////////////////////

    PapyroWindow::PapyroWindow(QWidget * parent, Qt::WindowFlags f)
        : Utopia::AbstractWindow(new PapyroWindowPrivate(this), parent, f)
    {
        U_D(PapyroWindow);
        d->initialise();
        d->emptyTab();
    }

    PapyroWindow::PapyroWindow(PapyroTab * tab, QWidget * parent, Qt::WindowFlags f)
        : Utopia::AbstractWindow(new PapyroWindowPrivate(this), parent, f)
    {
        U_D(PapyroWindow);
        d->initialise();
        d->addTab(tab);
    }

    PapyroWindow::~PapyroWindow()
    {
        U_D(PapyroWindow);

        clear();
    }

    void PapyroWindow::raiseWindow()
    {
        show();
        raise();
        activateWindow();
    }

    void PapyroWindow::clear()
    {
        U_D(PapyroWindow);

        // Delete tabs FIXME WRONG
        delete d->tabBar;
        d->tabBar = 0;

        setWindowTitle("Utopia Documents");
        setWindowFilePath(QString());
    }

    void PapyroWindow::closeAll()
    {
        boost::shared_ptr< Utopia::UIManager > manager(Utopia::UIManager::instance());
        QListIterator< PapyroWindow * > windows(manager->windows< PapyroWindow >());
        while (windows.hasNext()) {
            if (windows.hasPrevious()) {
                windows.next()->close();
            } else {
                windows.next()->clear();
            }
        }
    }

    void PapyroWindow::closeEvent(QCloseEvent * event)
    {
        // FIXME Do we need to ask the user if it's okay to close down?
        event->accept();
    }

    void PapyroWindow::dragEnterEvent(QDragEnterEvent * event)
    {
        U_D(PapyroWindow);
        if (event->source() == 0) { // External drag source
            QList< QUrl > supportedUrls(d->checkForSupportedUrls(event->mimeData()));
            if (!supportedUrls.isEmpty() || event->mimeData()->hasFormat("application/pdf")) {
                event->acceptProposedAction();
            }
        }
    }

    PapyroTab * PapyroWindow::currentTab() const
    {
        U_D(const PapyroWindow);
        return d->currentTab();
    }

    PapyroWindow * PapyroWindow::currentWindow()
    {
        PapyroWindow * active = 0;
        boost::shared_ptr< Utopia::UIManager > manager(Utopia::UIManager::instance());
        foreach (PapyroWindow * window, manager->windows< PapyroWindow >()) {
            if (active == 0 || window->isActiveWindow()) {
                active = window;
            }
        }
        return active;
        // FIXME should really show last active window
    }

    void PapyroWindow::dropEvent(QDropEvent * event)
    {
        U_D(PapyroWindow);

        if (event->source() == 0) {
            QList< QUrl > supportedUrls(d->checkForSupportedUrls(event->mimeData()));
            if (!supportedUrls.isEmpty()) {
                QList< QUrl > supportedUrls(d->checkForSupportedUrls(event->mimeData()));
                if (!supportedUrls.isEmpty()) {
                    event->acceptProposedAction();
                    QListIterator< QUrl > urls(supportedUrls);
                    while (urls.hasNext()) {
                        PapyroRecentUrlHelper::instance()->activateRecentUrl(urls.next());
                    }
                }
            }

            if (event->mimeData()->hasFormat("application/pdf")) {
                QByteArray data = event->mimeData()->data("application/pdf");
                QBuffer buffer(&data);
                open(&buffer);
                event->acceptProposedAction();
            }
        }
    }

    int PapyroWindow::indexOf(const QUrl & url) const
    {
        U_D(const PapyroWindow);
        for (int i = 0; i < d->tabLayout->count(); ++i) {
            if (PapyroTab * tab = qobject_cast< PapyroTab * >(d->tabLayout->widget(i))) {
                if (tab->url() == url) {
                    return i;
                }
            }
        }
        return -1;
    }

    int PapyroWindow::indexOf(PapyroTab * tab) const
    {
        U_D(const PapyroWindow);
        for (int i = 0; i < d->tabLayout->count(); ++i) {
            if (tab == qobject_cast< PapyroTab * >(d->tabLayout->widget(i))) {
                return i;
            }
        }
        return -1;
    }

    void PapyroWindow::keyPressEvent(QKeyEvent * event)
    {
        U_D(const PapyroWindow);
        if (event->key() == Qt::Key_Space) {
            if (PapyroTab * tab = d->currentTab()) {
                if (DocumentView * documentView = tab->documentView()) {
                    documentView->setExposing(true);
                }
            }
        }
    }

    void PapyroWindow::keyReleaseEvent(QKeyEvent * event)
    {
        U_D(const PapyroWindow);
        if (event->key() == Qt::Key_Space) {
            if (PapyroTab * tab = d->currentTab()) {
                if (DocumentView * documentView = tab->documentView()) {
                    documentView->setExposing(false);
                }
            }
        }
    }

    void PapyroWindow::modelSet()
    {
        U_D(PapyroWindow);
        if (model()) {
            Spine::DocumentHandle document(DocumentFactory::load(model()));
            if (document) {
                open(document);
            }
        }
    }

    PapyroWindow * PapyroWindow::newWindow()
    {
        return new PapyroWindow;
    }

    void PapyroWindow::open(Spine::DocumentHandle document, bool raise, const QVariantMap & params)
    {
        U_D(PapyroWindow);
        PapyroTab * tab = d->emptyTab();
        tab->setTitle("Loading...");
        if (raise) {
            raiseTab(d->tabBar->indexOf(tab));
        }
        tab->open(document, params);
    }

    void PapyroWindow::open(QIODevice * io, bool raise, const QVariantMap & params)
    {
        U_D(PapyroWindow);
        PapyroTab * tab = d->emptyTab();
        tab->setTitle("Loading...");
        if (raise) {
            raiseTab(d->tabBar->indexOf(tab));
        }
        tab->open(io, params);
    }

    void PapyroWindow::open(const QString & filename, bool raise, const QVariantMap & params)
    {
        U_D(PapyroWindow);
        PapyroTab * tab = d->emptyTab();
        tab->setTitle("Loading...");
        if (raise) {
            raiseTab(d->tabBar->indexOf(tab));
        }
        tab->open(filename, params);
    }

    void PapyroWindow::open(const QUrl & url, bool raise, const QVariantMap & params)
    {
        U_D(PapyroWindow);
        if (url.scheme() == "file") {
            open(url.toLocalFile());
        } else {
            PapyroTab * tab = d->emptyTab();
            if (raise) {
                raiseTab(d->tabBar->indexOf(tab));
            }
            tab->open(url, params);
            tab->setTitle("Fetching...");
        }
    }

    void PapyroWindow::openFile()
    {
        // Open file dialog for documents
        QPair< QString, Utopia::FileFormat* > toOpen = Utopia::getOpenFileName(this, QString("Open document"), QString(), Utopia::DocumentFormat);
        QString fileName = toOpen.first;
        if (!fileName.isEmpty() && QFileInfo(fileName).exists()) {
            //Utopia::FileFormat * format = toOpen.second;
            open(fileName);
        }
    }

    void PapyroWindow::openUrl()
    {
        U_D(PapyroWindow);
        bool ok;
        QString defaultUrl;
        QList< QUrl > clipboardUrls(d->checkForSupportedUrls(QApplication::clipboard()->mimeData()));
        if (!clipboardUrls.isEmpty()) {
            defaultUrl = clipboardUrls.first().toEncoded();
        }

        // Open URL dialog for documents
        QInputDialog input;
        input.setInputMode(QInputDialog::TextInput);
        input.setLabelText("URL:");
        input.setTextEchoMode(QLineEdit::Normal);
        input.setTextValue(defaultUrl);
        input.setWindowTitle("Enter the URL to the article you wish to open...");
        input.setMinimumWidth(500);
        input.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        input.resize(500, input.height());
        if (input.exec() == QDialog::Accepted && !input.textValue().isEmpty()) {
            QUrl url(QUrl::fromEncoded(input.textValue().toUtf8()));
            if (url.isValid()) {
                open(url);
            }
        }
    }

    void PapyroWindow::openFileFromClipboard()
    {
        U_D(PapyroWindow);
        foreach (const QUrl & url, d->checkForSupportedUrls(QApplication::clipboard()->mimeData())) {
            open(url);
        }
    }

    void PapyroWindow::raiseTab(int index)
    {
        U_D(PapyroWindow);
        d->tabBar->setCurrentIndex(index);
    }

    void PapyroWindow::requestUrl(const QUrl & url, const QString & target)
    {
        U_D(PapyroWindow);
        d->onUrlRequested(url, target);
    }

    void PapyroWindow::resizeEvent(QResizeEvent * event)
    {
        U_D(PapyroWindow);

        QMutableMapIterator< PapyroWindowPrivate::Layer, QPropertyAnimation * > iter(d->layerAnimations);
        while (iter.hasNext()) {
            iter.next();
            iter.value()->setEndValue(d->layerGeometryForState(iter.key(), d->toLayerState));
            if (d->layerAnimationGroup.state() == QAbstractAnimation::Stopped) {
                iter.value()->targetObject()->setProperty(iter.value()->propertyName(), iter.value()->endValue());
            } else {
                iter.value()->targetObject()->setProperty(iter.value()->propertyName(), iter.value()->currentValue());
            }
        }

        d->updateManualLayouts();
        Utopia::AbstractWindow::resizeEvent(event);
    }

    void PapyroWindow::saveFile()
    {
        U_D(PapyroWindow);
        if (PapyroTab * tab = d->currentTab()) {
            if (tab->documentView()->document()) {
                // Save file dialog for documents
                QSettings settings;
                settings.beginGroup("/File Dialogs");
                QString dir = settings.value("/lastVisitedDirectoryPath/Save").toString();
                QString fileName = QFileDialog::getSaveFileName(this, "Save PDF...", dir, "PDF Files (*.pdf)");
                std::string data(tab->documentView()->document()->data());
                if (!fileName.isEmpty()) {
                    QFile file(fileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(data.c_str(), data.size());
                    } else {
                        // FIXME can't write file?
                    }
                }
            }
        }
    }

    void PapyroWindow::searchRemote(const QString & text)
    {
        U_D(PapyroWindow);
        d->searchBox->search(text);
    }

    void PapyroWindow::showEvent(QShowEvent * event)
    {
        U_D(PapyroWindow);
        d->updateManualLayouts();
        Utopia::AbstractWindow::showEvent(event);
    }

    void PapyroWindow::showHelp()
    {
        QDesktopServices::openUrl(QUrl("http://utopiadocs.com/redirect.php?to=quickstart"));
    }

    void PapyroWindow::showAbout()
    {
        Utopia::AboutDialog * about = new Utopia::AboutDialog();
        about->setWindowModality(Qt::ApplicationModal);
        about->show();
        about->raise();
    }

    Utopia::Node * PapyroWindow::type() const
    {
        return Utopia::UtopiaDomain.term("Document");
    }

    void PapyroWindow::unsetModel()
    {
        U_D(PapyroWindow);
        QList< PapyroWindow * > windows = d->uiManager->windows< PapyroWindow >();
        if (windows.size() > 1) {
            close();
            // FIXME close sensibly
        }
    }

}
