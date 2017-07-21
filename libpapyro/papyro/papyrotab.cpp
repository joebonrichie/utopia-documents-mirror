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

#include <papyro/annotationprocessor.h>
#include <papyro/annotationprocessoraction.h>
#include <papyro/annotationresultitem.h>
#include <papyro/annotatorrunnable.h>
#include <papyro/annotatorrunnablepool.h>
#include <papyro/capabilities.h>
#include <papyro/citations.h>
#include <papyro/dispatcher.h>
#include <papyro/documentproxy.h>
#include <papyro/documentview.h>
#include <papyro/pager.h>
#include <papyro/papyrotab.h>
#include <papyro/papyrotab_p.h>
#include <papyro/progresslozenge.h>
#include <papyro/resultsview.h>
#include <papyro/searchbar.h>
#include <papyro/selectionprocessor.h>
#include <papyro/selectionprocessoraction.h>
#include <papyro/sidebar.h>
#include <papyro/utils.h>

#include <papyro/librarymodel.h>
#include <papyro/citation.h>

#include <utopia2/qt/flowbrowser.h>
#include <utopia2/qt/spinner.h>
#include <utopia2/qt/hidpi.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QNetworkReply>
#include <QPainter>
#include <QResizeEvent>
#include <QSplitterHandle>
#include <QStackedLayout>
#include <QVBoxLayout>

#include <QDebug>

namespace Papyro
{

    /// WidgetExpander ////////////////////////////////////////////////////////////////////////////

    static void sendResizeEvents(QWidget *target)
    {
        QResizeEvent e(target->size(), QSize());
        QApplication::sendEvent(target, &e);

        const QObjectList children = target->children();
        for (int i = 0; i < children.size(); ++i) {
            QWidget *child = dynamic_cast<QWidget*>(children.at(i));
            if (child && child->isWidgetType() && !child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent))
                sendResizeEvents(child);
        }
    }

    WidgetExpander::WidgetExpander(QWidget * child, QWidget * parent = 0)
        : QWidget(parent), _child(child), _period(200), _expanding(true), _oldHeight(0)
    {
        // Child and animation data
        QVBoxLayout * l = new QVBoxLayout(this);
        l->addWidget(_child);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSizeConstraint(QLayout::SetFixedSize);
        l->setSpacing(0);
        _childSizeHint = _child->sizeHint();
        _child->resize(_childSizeHint);
        _child->hide();
        connect(_child, SIGNAL(destroyed()), this, SLOT(close()));
        resize(_childSizeHint.width(), 0);

        _time.start();
        _timer.setInterval(60);
        _timer.start();
        connect(&_timer, SIGNAL(timeout()), this, SLOT(animate()));

        // Make sure this widget doesn't impinge on the UI
        setContentsMargins(0, 0, 0, 0);

        animate();
    }

    void WidgetExpander::animate()
    {
        if (_child && _oldHeight == 0) {
            _oldHeight = _child->height();
        }
        int targetHeight = _oldHeight < 0 ? 0 : _oldHeight;
        double progress = _time.elapsed() / (double) _period;

        //qDebug() << "animate()" << progress << targetHeight << height();
        // FInished?
        if (progress > 1.0) {
            if (_expanding) {
                if (targetHeight != height()) {
                    setMaximumHeight(targetHeight);
                    updateGeometry();
                    update();
                }

                if (progress > 2.0) {
                    // Set in a proper layout
                    _timer.stop();
                    _child->show();
                } else {
                    update();
                }
            } else {
                // Remove entirely
                _timer.stop();
                resize(width(), 0);
                deleteLater();
            }
        } else {
            // Animate
            setMaximumHeight((int) (targetHeight * (_expanding ? progress : 1 - progress)));
            updateGeometry();
            update();
        }
    }

    QWidget * WidgetExpander::child() const
    {
        return _child;
    }

    void WidgetExpander::close()
    {
        _expanding = false;
        _time.start();
        _timer.start();
        _child = 0;
    }

    bool WidgetExpander::event(QEvent * event)
    {
        //qDebug() << this << event;
        return QWidget::event(event);
    }

    void WidgetExpander::paintEvent(QPaintEvent * event)
    {
        //qDebug() << "WidgetExpander::paintEvent()" << _child << (_child ? _child->isHidden() : false);
        if (_child && _child->isHidden()) {
            int elapsed = _time.elapsed() - _period;
            if (elapsed > 0) {
                QPainter p(this);
                p.setOpacity(elapsed / (double) _period);
                QPixmap pixmap(rect().size());
                pixmap.fill(QColor(0, 0, 0, 0));
                if (_child->testAttribute(Qt::WA_PendingResizeEvent) || !_child->testAttribute(Qt::WA_WState_Created)) {
                    sendResizeEvents(_child);
                }
                _child->render(&pixmap, QPoint(), rect(), QWidget::DrawChildren | QWidget::IgnoreMask);
                p.drawPixmap(0, 0, pixmap);
            }
        }
    }

    int WidgetExpander::period() const
    {
        return _period;
    }

    void WidgetExpander::resizeEvent(QResizeEvent * event)
    {
        //qDebug() << "resizeEvent()" << event << event->size();
        animate();
    }

    void WidgetExpander::setPeriod(int period)
    {
        _period = period;
    }

    QSize WidgetExpander::sizeHint() const
    {
        //qDebug() << "----" << QSize(_childSizeHint.width(), maximumHeight());
        return QSize(_childSizeHint.width(), maximumHeight());
    }




    /// PapyroTabPrivate ////////////////////////////////////////////////////////////////

    PapyroTabPrivate::PapyroTabPrivate(PapyroTab * tab)
        : QObject(tab), tab(tab), progress(-1.0), state(PapyroTab::UninitialisedState),
          documentManager(DocumentManager::instance()), activeSelectionProcessorAction(0),
          ready(false)
    {
        // Create a new bus for this tab
        setBus(new Utopia::Bus(this));

        // Collect decorators
        foreach (Decorator * decorator, Utopia::instantiateAllExtensions< Decorator >()) {
            decorators.append(decorator);
        }

        libraryModel = Athenaeum::LibraryModel::instance();
    }

    PapyroTabPrivate::~PapyroTabPrivate()
    {
        cancelRunnables();

        // Delete decorator extensions
        while (!decorators.isEmpty()) {
            delete decorators.takeLast();
        }
    }

    void PapyroTabPrivate::cancelRunnables()
    {
        annotatorPool.skip();
        emit cancellationRequested();
        annotatorPool.waitForDone();
    }

    void PapyroTabPrivate::activateImage(int i)
    {
        documentView->showPage(imageAreas[i].page);
    }

    void PapyroTabPrivate::activateChemicalImage(int i)
    {
        Spine::TextExtentHandle extent(chemicalExtents.at(i));
        documentView->showPage(extent);
        documentView->hideSpotlights();
        Spine::TextSelection selection(extent);

        // Simulate a click for now (probably done differently eventually)
        PageView * pageView = documentView->pageView(extent->first.cursor()->page()->pageNumber());
        Spine::BoundingBox bb(extent->first.cursor()->word()->boundingBox());
        QPointF centerF(bb.x1 + bb.width() / 2.0, bb.y1 + bb.height() / 2.0);
        QPoint center(pageView->transformFromPage(centerF));
        QPoint globalCenter(pageView->mapToGlobal(center));
        QMouseEvent qm1(QEvent::MouseButtonPress, center, globalCenter, Qt::LeftButton , Qt::LeftButton,    Qt::NoModifier);
        QApplication::sendEvent(pageView, &qm1);
        QMouseEvent qm2(QEvent::MouseButtonRelease, center, globalCenter, Qt::LeftButton , Qt::LeftButton,    Qt::NoModifier);
        QApplication::sendEvent(pageView, &qm2);

        document()->setTextSelection(selection);
    }

    bool PapyroTabPrivate::on_activate_event_chain(boost::shared_ptr< Annotator > annotator, const QVariantMap & kwargs, QObject * obj, const char * receiver)
    {
        bool queued = false;

        // Set off the event handlers
        queued = handleEvent(annotator, "activate", kwargs);
        queued = queued && handleEvent("filter", kwargs, obj, receiver);

        return queued;
    }

    QString PapyroTabPrivate::busId() const
    {
        static QString id("papyro.queue");
        return id;
    }

    Spine::DocumentHandle PapyroTabPrivate::document() const
    {
        // Access the Spine Document object for this tab if one exists
        if (documentView) {
            return documentView->document();
        } else {
            return Spine::DocumentHandle();
        }
    }

    bool PapyroTabPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        if (obj) {
            QSplitterHandle * contentSplitterHandle = dynamic_cast< QSplitterHandle * >(obj);
            if (obj == documentView) {
                if (event->type() == QEvent::Move || event->type() == QEvent::Resize) {
                    quickSearchBar->move(documentView->mapTo(tab, QPoint(0, 0)) + QPoint(20, 0));
                    quickSearchBar->setFixedWidth(documentView->width() - 40);
                }
            } else if (contentSplitterHandle) {
                if (event->type() == QEvent::Paint) {
                    QRect rect(contentSplitterHandle->rect());
                    QPainter painter(contentSplitterHandle);
                    if (rect.width() > rect.height()) {
                        painter.setBrush(painter.pen().color());
                        painter.drawRect(rect.adjusted(0, 1, 0, 1));
                    }
                    return true;
                } else {
                    return false;
                }
            } else if (obj == lookupWidget) {
                if (event->type() == QEvent::Show) {
                    lookupTextBox->setFocus(Qt::TabFocusReason);
                    return true;
                } else {
                    return false;
                }
            } else if (obj == progressWidget) {
                if (event->type() == QEvent::Paint) {
                    QPainter painter(progressWidget);
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.setBrush(Qt::white);
                    painter.setPen(Qt::NoPen);
                    painter.drawRect(progressWidget->rect());
                    if (documentView->isEmpty() && state == PapyroTab::EmptyState) {
                        painter.setOpacity(0.04);
                        QSize size = watermarkRenderer.defaultSize();
                        size.scale(400, 400, Qt::KeepAspectRatio);
                        QRect sRect(QPoint(0, 0), size);
                        sRect.moveCenter(progressWidget->rect().center());
                        QPixmap overlay(sRect.size());
                        overlay.fill(QColor(0, 0, 0, 0));
                        QPainter overlayPainter(&overlay);
                        watermarkRenderer.render(&overlayPainter, QRect(QPoint(0, 0), sRect.size()));
                        painter.drawPixmap(sRect.topLeft(), overlay);
                    }
                    return true;
                } else {
                    return false;
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

    void PapyroTabPrivate::executeAnnotator(int idx)
    {
        Spine::DocumentHandle doc = document();

        if (doc && activatableAnnotators.size() > idx) {
            setState(PapyroTab::ProcessingState);
            on_activate_event_chain(activatableAnnotators.at(idx));
        }
    }

    void PapyroTabPrivate::exploreSelection()
    {
        // Specifically, this method does not show you annotations under the mouse!
        Spine::TextSelection selection = document()->textSelection();
        //qDebug() << "PapyroTabPrivate::exploreSelection()" << selection.size();

        // Include the current text selection
        if (!selection.empty()) {
            QStringList terms;
            foreach (const Spine::TextExtentHandle & extent, selection) {
                QRegExp stripRE("^\\W*(\\w.*\\w|\\w)\\W*$");
                QString term(qStringFromUnicode(extent->text()));
                term.replace(stripRE, "\\1");
                terms.append(term);
            }
            terms.removeDuplicates();

            actionToggleSidebar->setChecked(true);
            sidebar->setMode(Sidebar::Results);
            sidebar->resultsView()->clear();
            sidebar->setSearchTerm(terms.join(", "));
            dispatcher->lookupOLD(document(), terms);
        }
    }

    void PapyroTabPrivate::focusChanged(PageView * pageView, QPointF pagePos)
    {
    }

    bool PapyroTabPrivate::handleEvent(const QString & event, const QVariantMap & kwargs, QObject * obj, const char * receiver)
    {
        bool queued = false;

        if (event.contains(':')) {
            bool parallel = event.startsWith("on:");
            QMapIterator< int, QList< boost::shared_ptr< Annotator > > > iter(eventHandlers.value(event));
            while (iter.hasNext()) {
                iter.next();
                foreach (boost::shared_ptr< Annotator > annotator, iter.value()) {
                    AnnotatorRunnable * runnable = new AnnotatorRunnable(annotator, event, document(), kwargs);
                    connect(this, SIGNAL(cancellationRequested()), runnable, SLOT(cancel()));
                    runnable->setAutoDelete(false);
                    queueAnnotatorRunnable(runnable);
                    queued = true;
                    if (!parallel) {
                        annotatorPool.sync();
                    }
                }
                if (parallel) {
                    annotatorPool.sync();
                }
            }
            if (obj && receiver) {
                annotatorPool.sync(obj, receiver);
            } else {
                annotatorPool.sync();
            }
        } else {
            queued = handleEvent("before:"+event, kwargs) || queued;
            queued = handleEvent("on:"+event, kwargs) || queued;
            queued = handleEvent("after:"+event, kwargs, obj, receiver) || queued;
        }

        if (event == "filter") {
            annotatorPool.sync(this, SLOT(onFilterFinished()));
        }

        return queued;
    }

    bool PapyroTabPrivate::handleEvent(boost::shared_ptr< Annotator > annotator, const QString & event, const QVariantMap & kwargs, QObject * obj, const char * receiver)
    {
        bool queued = false;

        if (event.contains(':')) {
            AnnotatorRunnable * runnable = new AnnotatorRunnable(annotator, event, document(), kwargs);
            connect(this, SIGNAL(cancellationRequested()), runnable, SLOT(cancel()));
            runnable->setAutoDelete(false);
            queueAnnotatorRunnable(runnable);
            queued = true;
            if (obj && receiver) {
                annotatorPool.sync(obj, receiver);
            } else {
                annotatorPool.sync();
            }
        } else {
            queued = handleEvent(annotator, "before:"+event, kwargs) || queued;
            queued = handleEvent(annotator, "on:"+event, kwargs) || queued;
            queued = handleEvent(annotator, "after:"+event, kwargs, obj, receiver) || queued;
        }

        return queued;
    }

    void PapyroTabPrivate::loadAnnotators()
    {
        if (!ready) {
            // Collect all annotator event handlers
            foreach (Annotator * ann, Utopia::instantiateAllExtensions< Annotator >()) {
                boost::shared_ptr< Annotator > annotator(ann);
                bool used = false;

                if (annotator->canHandleEvent("on:explore")) {
                    lookups.append(annotator);
                    used = true;
                }

                // Get event handlers
                foreach (const QString & eventSpec, annotator->handleableEvents()) {
                    static QRegExp parse("(\\w+:\\w+)(?:/(-?\\d+))?");
                    if (parse.exactMatch(eventSpec)) {
                        QString name(parse.cap(1));
                        int weight(parse.cap(2).toInt());
                        eventHandlers[name][weight] << annotator;
                        if (name == "on:activate") {
                            activatableAnnotators << annotator;
                        }
                        used = true;
                    }
                }

                if (used) {
                    annotator->setBus(bus());
                    annotators.append(annotator);
                }
            }
            // Pass plugins around to sub-components
            dispatcher->setLookups(lookups);

            // Initialise annotators
            handleEvent("init");

            ready = true;
        }
    }

    void PapyroTabPrivate::loadChemicalImage(int i)
    {
    }

    void PapyroTabPrivate::loadImage(int i)
    {
        Spine::Image image(document()->renderArea(imageAreas[i], 100));
        imageBrowserModel->update(i, qImageFromSpineImage(&image));
    }

    void PapyroTabPrivate::loadNextPagerImage()
    {
        if (document()) {
            if (pagerQueue.isEmpty()) {
                pagerTimer.stop();
            } else {
                int i = pagerQueue.dequeue();
                PageView * pageView = documentView->pageView(i+1);
                const Spine::Page * page = document()->newCursor(i+1)->page();
                QSize size = pageView->pageSize().toSize();
                size.scale(QSize(120, 120), Qt::KeepAspectRatio);
                Spine::Image spineImage = page->render(size_t(size.width()),
                                                       size_t(size.height()));
                pager->replace(i, QPixmap::fromImage(qImageFromSpineImage(&spineImage)).transformed(pageView->userTransform()));
            }
        } else {
            pagerQueue.clear();
        }
    }

    bool PapyroTabPrivate::on_marshal_event_chain(QObject * obj, const char * receiver)
    {
        bool queued = false;

        // Start by setting off the event handlers
        queued = handleEvent("marshal");
        queued = handleEvent("persist", QVariantMap(), obj, receiver) || queued;

        return queued;
    }

    void PapyroTabPrivate::onAnnotatorFinished()
    {
#ifdef UTOPIA_BUILD_DEBUG
        AnnotatorRunnable * runnable = qobject_cast< AnnotatorRunnable * >(sender());
        qDebug() << "Runnable FINISHED:" << runnable->title();
#endif
        --activeAnnotators;

        if (activeAnnotators == 0) {
            statusWidgetTimer.stop();
        }
    }

    void PapyroTabPrivate::onAnnotatorSkipped()
    {
#ifdef UTOPIA_BUILD_DEBUG
        AnnotatorRunnable * runnable = qobject_cast< AnnotatorRunnable * >(sender());
        qDebug() << "Runnable SKIPPED:" << runnable->title();
#endif
    }

    void PapyroTabPrivate::onAnnotatorStarted()
    {
#ifdef UTOPIA_BUILD_DEBUG
        AnnotatorRunnable * runnable = qobject_cast< AnnotatorRunnable * >(sender());
        qDebug() << "Runnable STARTED:" << runnable->title();
#endif
        if (activeAnnotators == 1) {
            statusWidgetTimer.start();
        }
    }

    void PapyroTabPrivate::onCitationChanged(int index, QVariant oldValue)
    {
        if (citation) {
            bool isStarred = citation->isStarred();
            bool isKnown = citation->isKnown();

            if (index == Athenaeum::Citation::FlagsRole) {
                //bool wasStarred = (oldValue.value< Athenaeum::Citation::Flags >() & Athenaeum::Citation::StarredFlag);
                if (isStarred && !isKnown) {
                    emit starredChanged(isStarred);
                }
            }
            if (index == Athenaeum::Citation::KnownRole && citation) {
                bool wasKnown = oldValue.toBool();
                if (wasKnown != isKnown) {
                    emit knownChanged(isKnown);
                }
            }
        }
    }

    void PapyroTabPrivate::onDispatcherAnnotationFound(Spine::AnnotationHandle annotation)
    {
        if (annotation->capable< SummaryCapability >()) {
            sidebar->resultsView()->addResult(new AnnotationResultItem(annotation));
        }
    }

    std::vector< std::string > weightedProperty(const Spine::AnnotationSet & annotations, const std::string & key, const std::map< std::string, std::string > & criteria = std::map< std::string, std::string >())
    {
        typedef std::map< std::string, std::string > string_map;

        QMap< int, Spine::AnnotationSet > weighted;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            bool matches = true;
            string_map::const_iterator c_iter(criteria.begin());
            string_map::const_iterator c_end(criteria.end());
            while (matches && c_iter != c_end) {
                if (c_iter->second.empty()) {
                    matches = matches && annotation->hasProperty(c_iter->first);
                } else {
                    matches = matches && annotation->hasProperty(c_iter->first, c_iter->second);
                }
                ++c_iter;
            }
            if (matches && annotation->hasProperty(key)) {
                int weight = qStringFromUnicode(annotation->getFirstProperty("session:weight")).toInt();
                weighted[weight].insert(annotation);
            }
        }

        // Set new authors
        if (!weighted.isEmpty()) {
            return (*--(--weighted.end())->end())->getProperty(key);
        } else {
            return std::vector< std::string >();
        }
    }

    std::string weightedFirstProperty(const Spine::AnnotationSet & annotations, const std::string & key, const std::map< std::string, std::string > & criteria)
    {
        std::vector< std::string > values(weightedProperty(annotations, key, criteria));
        if (values.empty()) {
            return std::string();
        } else {
            return values[0];
        }
    }

    void PapyroTabPrivate::onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added)
    {
        if (document()) {
            if (name.empty()) {
                QMap< int, int > pageMarkers;
                for (size_t page = 1; page <= document()->numberOfPages(); ++page) {
                    pageMarkers[page-1] = 0;
                    foreach (Spine::AnnotationHandle annotation, document()->annotationsAt(page)) {
                        if (!annotation->hasProperty("session:volatile")) {
                            pageMarkers[page-1] += 1;
                            break; // FIXME this ignores cardinality
                        }
                    }
                }
                pager->setAnnotations(pageMarkers);

                if (added) {
                    foreach (Spine::AnnotationHandle annotation, annotations) {
                        // Give each a cssId
                        annotation->setProperty("session:cssId", unicodeFromQString(QString("result-") + QString("000000000000%1").arg(qrand()).right(8)));

                        foreach (Decorator * decorator, decorators) {
                            foreach (Spine::CapabilityHandle capability, decorator->decorate(annotation)) {
                                annotation->addCapability(capability);
                            }
                        }
                    }

                    // Document wide data
                    BOOST_FOREACH (Spine::AnnotationHandle annotation, annotations) {
                        if (annotation->areas().size() == 0 && annotation->extents().size() == 0) {
                            if (annotation->capable< SummaryCapability >()) {
                                actionToggleSidebar->setChecked(true);
                                sidebar->documentWideView()->addResult(new AnnotationResultItem(annotation));
                            }
                        }
                    }
                }
            } else if (name == "Chemicals") {
                BOOST_FOREACH(Spine::AnnotationHandle annotation, annotations)
                {
                    QImage thumbnail(QImage::fromData(QByteArray::fromBase64(qStringFromUnicode(annotation->getFirstProperty("property:thumbnail")).toUtf8())));
                    chemicalExtents.append(*annotation->extents().begin());
                    chemicalBrowserModel->append("", thumbnail);
                    imageBrowser->setCurrentModel(chemicalBrowserModel);
                }
            } else if (name == "Document Metadata") {
                typedef std::map< std::string, std::string > string_map;
                string_map criteria;
                QString tabTitle("Unknown ");
                Spine::AnnotationSet anns(document()->annotations(name));

                // Find the first author's surname and the year
                criteria["concept"] = "Citation";
                QStringList authors;
                foreach (const std::string & author, weightedProperty(anns, "property:authors", criteria)) {
                    QString surname = qStringFromUnicode(author).section(',', 0, 0);
                    authors << surname;
                }
                if (!authors.isEmpty()) {
                    tabTitle = authors.first() + " ";
                }

                QString year = qStringFromUnicode(weightedFirstProperty(anns, "property:year", criteria));
                if (!year.isEmpty()) {
                    tabTitle += "(" + year + ") ";
                }

                QString title = qStringFromUnicode(weightedFirstProperty(anns, "property:title", criteria));
                if (title.isEmpty()) {
                    criteria["concept"] = "Citation";
                    title = qStringFromUnicode(weightedFirstProperty(anns, "property:title", criteria));
                }
                if (title.isEmpty()) {
                    title = "Unknown Document";
                } else {
                    title = "\"" + title + "\"";
                }

                if (!tabTitle.isEmpty()) {
                    tabTitle += "- ";
                }
                tabTitle += title;

                // Set new title
                tabTitle = tabTitle.trimmed();
                if (!tabTitle.isEmpty()) {
                    tab->setTitle(tabTitle);
                }

                // Check whether we have any identifiers available
                if (added && !citation) {
                    Athenaeum::Bibliography * master = libraryModel->master();
                    Athenaeum::CitationHandle item;
                    foreach (Spine::AnnotationHandle annotation, annotations) {
                        if (annotation->getFirstProperty("concept") == "Citation" &&
                            annotation->hasProperty("property:identifiers")) {
                            std::string value = annotation->getFirstProperty("property:identifiers");
                            if (value.substr(0, 5) == "json:") {
                                std::string json(value.substr(5));
                                QVariantMap identifiers = QJsonDocument::fromJson(json.c_str()).toVariant().toMap();
                                QMapIterator< QString, QVariant > iter(identifiers);
                                while (iter.hasNext()) {
                                    iter.next();
                                    QString id(iter.key() + ":" + iter.value().toString());
                                    if ((item = master->itemForId(id))) {
                                        break;
                                    }
                                }
                                if (item) {
                                    break;
                                }
                            }
                        }
                    }
                    if (item) {
                        tab->setCitation(item);
                    }
                }
            } else if (document() && (name == "PersistQueue" || name == document()->deletedItemsScratchId()) && added) {
                qDebug() << "+++++++ PERSIST";
                publishChanges();
            }
        }
    }

    void PapyroTabPrivate::onDocumentAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added)
    {
        //qDebug() << "onDocumentAreaSelectionChanged" << added << QString::fromStdString(name) << areas.size();
        sidebar->onSelectionChanged();
        if (added && name.empty() && activeSelectionProcessorAction) {
             activeSelectionProcessorAction->trigger();
        }
    }

    void PapyroTabPrivate::onDocumentProcessingFinished()
    {
        // If at this point, the citation for this document is still null, then
        // we can be certain it will not be found in the library. We should
        // therefore generate a new Citation with all the correct metadata.

        // FIXME this needs to copy ALL the metadata, not just a few things

        if (!citation) {
            Athenaeum::CitationHandle candidate = Athenaeum::CitationHandle(new Athenaeum::Citation);
            Spine::AnnotationSet annotations(document()->annotations("Document Metadata"));
            QVariantMap collated;
            QMap< QString, int > weights;
            foreach (Spine::AnnotationHandle annotation, annotations) {
                if (annotation->getFirstProperty("concept") == "Citation") {
                    QVariantMap mapping(citationToMap(annotation));
                    int weight = 0;
                    if (mapping.contains(":weight")) {
                        weight = mapping.value("weight").toInt();
                    }
                    QMapIterator< QString, QVariant > iter(mapping);
                    while (iter.hasNext()) {
                        iter.next();
                        if (iter.key() == "identifiers") {
                            QVariantMap new_ids = iter.value().toMap();
                            QVariantMap old_ids = collated[iter.key()].toMap();
                            QMapIterator< QString, QVariant > id_iter(new_ids);
                            while (id_iter.hasNext()) {
                                id_iter.next();
                                if (!old_ids.contains(id_iter.key())) {
                                    old_ids[id_iter.key()] = id_iter.value();
                                }
                            }
                            collated[iter.key()] = old_ids;
                        } else if (iter.key() == "links") {
                            QVariantList new_links = iter.value().toList();
                            QVariantList old_links = collated[iter.key()].toList();
                            QListIterator< QVariant > link_iter(new_links);
                            while (link_iter.hasNext()) {
                                QVariantList link = link_iter.next().toList();
                                if (!old_links.contains(link)) {
                                    old_links.append(link);
                                }
                            }
                            collated[iter.key()] = old_links;
                        } else {
                            if (!weights.contains(iter.key()) || weight > weights.value(iter.key(), 0)) {
                                weights[iter.key()] = weight;
                                collated[iter.key()] = iter.value();
                            }
                        }
                    }
                }
            }
            QMapIterator< QString, QVariant > iter(collated);
            while (iter.hasNext()) {
                iter.next();
                int role = -1;
                if (iter.key() == "title" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::TitleRole; }
                else if (iter.key() == "authors" && !iter.value().toList().isEmpty()) { role = Athenaeum::Citation::AuthorsRole; }
                else if (iter.key() == "year" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::YearRole; }
                else if (iter.key() == "issue" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::IssueRole; }
                else if (iter.key() == "volume" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::VolumeRole; }
                else if (iter.key() == "identifiers" && !iter.value().toMap().isEmpty()) { role = Athenaeum::Citation::IdentifiersRole; }
                else if (iter.key() == "publication-title" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::PublicationTitleRole; }
                else if (iter.key() == "abstract" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::AbstractRole; }
                else if (iter.key() == "publisher" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::PublisherRole; }
                else if (iter.key() == "keywords" && !iter.value().toString().isEmpty()) { role = Athenaeum::Citation::KeywordsRole; }
                else if (iter.key() == "links" && !iter.value().toList().isEmpty()) { role = Athenaeum::Citation::LinksRole; }

                if (role >= 0) {
                    candidate->setField(role, iter.value());
                }
            }
            tab->setCitation(candidate);
        }

        // What if this article was launched from a citation for which no PDF
        // had been saved? If we now have a PDF, it should be saved into the
        // library.
        if (citation && citation->isKnown() && !libraryModel->hasObjectFile(citation, ".pdf")) {
            std::string str(document()->data());
            QByteArray data(str.c_str(), str.size());
            libraryModel->saveObjectFile(citation, data, ".pdf");
        }
    }

    void PapyroTabPrivate::onDocumentTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added)
    {
        //qDebug() << "onDocumentTextSelectionChanged" << added << QString::fromStdString(name) << QString::fromStdString((*extents.begin())->text()) << extents.size();
        sidebar->onSelectionChanged();
        if (added && name.empty() && activeSelectionProcessorAction) {
             activeSelectionProcessorAction->trigger();
        }
    }

    void PapyroTabPrivate::onDocumentViewAnnotationsActivated(Spine::AnnotationSet annotations, const QVariantMap & context)
    {
        QPoint globalPos(context.value("globalPos").toPoint());

        // Find a suitable processor for this annotation
        QMap< int, QMap< int, QMap< QString, QList< AnnotationProcessor * > > > > ordered;
        foreach(AnnotationProcessor * processor, annotationProcessors) {
            QList< Spine::AnnotationSet > batches(processor->canActivate(document(), annotations));
            if (!batches.empty() && !processor->title(document(), batches.first()).contains("|")) {
                ordered[processor->category()][processor->weight()][title] << processor;
            }
        }
        if (!ordered.isEmpty()) {
            ordered.begin().value().begin().value().begin().value().first()->activate(document(), annotations, globalPos);
        } else {
            // What if no suitable processor was found? Bung in the sidebar :)
            visualiseAnnotations(annotations, context);
        }
    }

    void PapyroTabPrivate::onDocumentViewContextMenu(QMenu * menu, Spine::DocumentHandle document, Spine::CursorHandle cursor)
    {
        static QRegExp sep("\\s*\\|\\s*");

        // Keep track of actions already inserted
        QList< QAction * > actions(menu->actions());
        QAction * first = actions.isEmpty() ? 0 : actions.first();

        Spine::AnnotationSet annotations = documentView->activeAnnotations();
        if (!annotations.empty()) {
            // Order by path / category / weight / title
            QMap< QString, QMap< int, QMap< int, QMap< QString, QList< QAction * > > > > > ordered;
            QMap< QString, QMenu * > submenus;
            submenus[QString()] = submenus[QString("")] = menu;

            foreach(AnnotationProcessor * processor, annotationProcessors) {
                QList< Spine::AnnotationSet > batches(processor->canActivate(document, annotations));
                if (!batches.empty()) {
                    QString title = processor->title(document, batches.first());
                    QString path(title.trimmed().section(sep, 0, -2, QString::SectionSkipEmpty).replace(sep, "|"));
                    title = title.trimmed().section(sep, -1, -1, QString::SectionSkipEmpty).trimmed().toLower();
                    QAction * action = new AnnotationProcessorAction(processor, document, batches.first());
                    ordered[path][processor->category()][processor->weight()][title] << action;
                }
            }

            // Go through the ordered actions and populate the menu accordingly
            QMapIterator< QString, QMap< int, QMap< int, QMap< QString, QList< QAction * > > > > > p_iter(ordered);
            while (p_iter.hasNext()) {
                p_iter.next();

                // Ensure there's a menu to add these actions to
                QString path(p_iter.key().toLower());
                if (!submenus.contains(path)) {
                    for (int parts = path.count(sep); parts >= 0; --parts) {
                        QString base = path.section(sep, 0, parts, QString::SectionSkipEmpty);
                        if (!submenus.contains(base)) {
                            submenus[base] = new QMenu(p_iter.key().section(sep, parts, parts, QString::SectionSkipEmpty));
                        }
                    }
                }
                QMenu * curr = submenus[path];

                // Place actions accordingly
                QMapIterator< int, QMap< int, QMap< QString, QList< QAction * > > > > c_iter(p_iter.value());
                while (c_iter.hasNext()) {
                    c_iter.next();
                    QMapIterator< int, QMap< QString, QList< QAction * > > > w_iter(c_iter.value());
                    while (w_iter.hasNext()) {
                        w_iter.next();
                        QMapIterator< QString, QList< QAction * > > t_iter(w_iter.value());
                        while (t_iter.hasNext()) {
                            t_iter.next();
                            foreach (QAction * action, t_iter.value()) {
                                curr->insertAction(first, action);
                                if (curr == menu && curr->defaultAction() == 0) {
                                    curr->setDefaultAction(action);
                                }
                            }
                        }
                    }
                    curr->insertSeparator(first);
                }
            }

            // Link hierarchy of menus accordingly
            QMapIterator< QString, QMenu * > sm_iter(submenus);
            while (sm_iter.hasNext()) {
                sm_iter.next();
                if (!sm_iter.key().isEmpty()) {
                    QString parent = sm_iter.key().section(sep, 0, -2, QString::SectionSkipEmpty);
                    if (submenus.contains(parent)) {
                        submenus[parent]->insertMenu(first, sm_iter.value());
                    }
                }
            }
        }

        {
            // Order by path / category / weight / title
            QMap< QString, QMap< int, QMap< int, QMap< QString, QList< SelectionProcessorAction * > > > > > ordered;
            QMap< QString, QMenu * > submenus;
            submenus[QString()] = submenus[QString("")] = menu;

            // Can we create new annotations?
            foreach(SelectionProcessorFactory * processorFactory, selectionProcessorFactories) {
                QList< boost::shared_ptr< SelectionProcessor > > processors = processorFactory->selectionProcessors(document, cursor);
                foreach (boost::shared_ptr< SelectionProcessor > processor, processors) {
                    QString path(processor->title().trimmed().section(sep, 0, -2, QString::SectionSkipEmpty).replace(sep, "|"));
                    QString title(processor->title().trimmed().section(sep, -1, -1, QString::SectionSkipEmpty).trimmed().toLower());
                    ordered[path][processor->category()][processor->weight()][title] << new SelectionProcessorAction(processor, document, cursor);
                }
            }

            // Go through the ordered actions and populate the menu accordingly
            QMapIterator< QString, QMap< int, QMap< int, QMap< QString, QList< SelectionProcessorAction * > > > > > p_iter(ordered);
            while (p_iter.hasNext()) {
                p_iter.next();

                // Ensure there's a menu to add these actions to
                QString path(p_iter.key().toLower());
                if (!submenus.contains(path)) {
                    for (int parts = path.count(sep); parts >= 0; --parts) {
                        QString base = path.section(sep, 0, parts, QString::SectionSkipEmpty);
                        if (!submenus.contains(base)) {
                            submenus[base] = new QMenu(p_iter.key().section(sep, parts, parts, QString::SectionSkipEmpty));
                        }
                    }
                }
                QMenu * curr = submenus[path];

                // Place actions accordingly
                QMapIterator< int, QMap< int, QMap< QString, QList< SelectionProcessorAction * > > > > c_iter(p_iter.value());
                while (c_iter.hasNext()) {
                    c_iter.next();
                    QMapIterator< int, QMap< QString, QList< SelectionProcessorAction * > > > w_iter(c_iter.value());
                    while (w_iter.hasNext()) {
                        w_iter.next();
                        QMapIterator< QString, QList< SelectionProcessorAction * > > t_iter(w_iter.value());
                        while (t_iter.hasNext()) {
                            t_iter.next();
                            foreach (SelectionProcessorAction * processorAction, t_iter.value()) {
                                curr->insertAction(first, processorAction);
                            }
                        }
                    }
                    curr->insertSeparator(first);
                }
            }

            // Link hierarchy of menus accordingly
            QMapIterator< QString, QMenu * > sm_iter(submenus);
            while (sm_iter.hasNext()) {
                sm_iter.next();
                if (!sm_iter.key().isEmpty()) {
                    QString parent = sm_iter.key().section(sep, 0, -2, QString::SectionSkipEmpty);
                    if (submenus.contains(parent)) {
                        submenus[parent]->insertMenu(first, sm_iter.value());
                    }
                }
            }
        }

        menu->insertSeparator(first);

        menu->addSeparator();

        // Add document annotation functionality
        if (activatableAnnotators.size() > 0) {
            menu->addSeparator();

            QMap< QString, int > idxMap;
            int j = 0;
            foreach (boost::shared_ptr< Annotator > annotator, activatableAnnotators) {
                // Only add annotators that have dynamic invocations
                QString title = qStringFromUnicode(annotator->title());
                if (!title.startsWith("!")) {
                    idxMap[title] = j;
                }

                ++j;
            }

            if (idxMap.size() > 0) {
                QMenu * subMenu = menu->addMenu("Annotate Document");

                QMapIterator< QString, int > idxMaps(idxMap);
                while (idxMaps.hasNext()) {
                    idxMaps.next();
                    boost::shared_ptr< Annotator > annotator = activatableAnnotators.at(idxMaps.value());
                    annotatorMapper->setMapping(subMenu->addAction(idxMaps.key(), annotatorMapper, SLOT(map())), idxMaps.value());
                }
            }
        }

        emit contextMenuAboutToShow(menu);
    }

/*
    void PapyroTabPrivate::onDocumentViewManageSelection(Spine::AreaSet areas)
    {
        selectionManager->populate(areas);
    }

    void PapyroTabPrivate::onDocumentViewManageSelection(Spine::TextSelection selection, bool expand)
    {
        Spine::TextExtentSet extents = selection;
        if (expand)
        {
            QSet< QString > terms;
            BOOST_FOREACH(Spine::TextExtentHandle extent, extents)
            {
                std::string term(extent->text());
                if (!term.empty())
                {
                    terms << qStringFromUnicode(term);
                }
            }
            extents.clear();
            QSetIterator< QString > term(terms);
            while (term.hasNext())
            {
                Spine::TextExtentSet results(document()->search(unicodeFromQString(term.next())));
                extents.insert(results.begin(), results.end());
            }
        }
        selectionManager->populate(extents);
    }
*/

    void PapyroTabPrivate::onDocumentViewPageFocusChanged(size_t pageNumber)
    {
        pager->focus(pageNumber - 1);
    }

    void PapyroTabPrivate::onDocumentViewSpotlightsHidden()
    {
        quickSearchBar->hide();
    }

    void PapyroTabPrivate::onFilterFinished()
    {
        //if (!annotatorPool.isActive()) {
            setState(PapyroTab::IdleState);
        //}
    }

    void PapyroTabPrivate::onImageBrowserEmptinessChanged(bool empty)
    {
        actionToggleImageBrowser->setDisabled(empty);
        if (empty) {
            actionToggleImageBrowser->setChecked(false);
            actionToggleImageBrowser->setToolTip("No figures found");
        } else {
            actionToggleImageBrowser->setToolTip("Toggle Figure Browser");
        }
    }

    void PapyroTabPrivate::onLookupOverride()
    {
        actionToggleSidebar->setChecked(true);
        QString term = lookupTextBox->text();
        if (!term.isEmpty())
        {
            sidebar->setMode(Sidebar::Results);
            sidebar->resultsView()->clear();
            actionToggleSidebar->setChecked(true);
            sidebar->setSearchTerm(term);
            dispatcher->lookupOLD(document(), term);
        }
    }

    void PapyroTabPrivate::explore(const QString & term)
    {
        if (!term.isEmpty()) {
            dispatcher->lookupOLD(document(), term);
        }
    }

    void PapyroTabPrivate::onLookupStarted()
    {
        if (lookupButton->text() == "Explore") {
            lookupButton->setFixedWidth(lookupButton->width());
            lookupButton->setText("Cancel");
            disconnect(lookupButton, SIGNAL(clicked()), this, SLOT(onLookupOverride()));
            connect(lookupButton, SIGNAL(clicked()), dispatcher, SLOT(clear()));
        }
    }

    void PapyroTabPrivate::onLookupStopped()
    {
        if (lookupButton->text() == "Cancel") {
            lookupButton->setText("Explore");
            disconnect(lookupButton, SIGNAL(clicked()), dispatcher, SLOT(clear()));
            connect(lookupButton, SIGNAL(clicked()), this, SLOT(onLookupOverride()));
        }
    }

    void PapyroTabPrivate::onNetworkReplyFinished()
    {
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
        reply->deleteLater();

        QVariant redirectsVariant = reply->property("__redirects");
        Athenaeum::CitationHandle citation = reply->property("__originalCitation").value< Athenaeum::CitationHandle >();
        QVariantMap params = reply->property("__originalParams").toMap();
        QString expects = reply->property("__originalExpects").toString();
        QList< QVariant > fallbackLinks = reply->property("__fallbackLinks").toList();
        QVariantMap link = reply->property("__thisLink").toMap();
        int redirects = redirectsVariant.isNull() ? 20 : redirectsVariant.toInt();
        QString error;

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
                QNetworkReply * reply = networkAccessManager()->get(request);
                reply->setProperty("__redirects", redirects - 1);
                reply->setProperty("__originalCitation", QVariant::fromValue(citation));
                reply->setProperty("__originalExpects", expects);
                reply->setProperty("__originalParams", params);
                reply->setProperty("__thisLink", link);
                reply->setProperty("__fallbackLinks", fallbackLinks);
                connect(reply, SIGNAL(finished()), this, SLOT(onNetworkReplyFinished()));
                connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
                return;
            } else {
                // Too many times
                error = "The document URL redirected too many times, so I've abandoned the attempt.";
            }
        } else {
            if (reply->error() == QNetworkReply::NoError) {
                // Get various attributes of the reply
                QUrl url(reply->request().url());
                QString contentType(reply->header(QNetworkRequest::ContentTypeHeader).toString());
                QString mimeType(contentType.section(';', 0, 0).trimmed().toLower());

                // Work out what type of reply this is
                bool expectsPdf = (expects == "application/pdf");
                bool isFile = (url.scheme() == "file");
                bool isOctetStream = (mimeType == "application/octet-stream");
                bool isPdf = ((mimeType == "application/pdf") || (expectsPdf && isOctetStream));
                bool isAccepted = (mimeType == expects);

                // Check whether the reply is a PDF attachment
                bool isPdfAttachment = false;
                QString contentDisposition(QString::fromUtf8(reply->rawHeader("Content-Disposition")));
                if (!contentDisposition.isEmpty()) {
                    foreach (QString param, contentDisposition.split(";")) {
                        if (param.section("=", 0, 0).trimmed() == "filename" &&
                            param.section("=", -1, -1).trimmed().endsWith(".pdf")) {
                            isPdfAttachment = true;
                            break;
                        }
                    }
                }

                // If we can open this reply in the tab, do so
                if (isFile || isPdfAttachment || isPdf) {
                    tab->open(reply, params, citation);
                } else if (isAccepted) {
                    // If it's still accepted, but can't be opened directly,
                    // hand it over to the OS
                    QDesktopServices::openUrl(reply->request().url());
                    emit closeRequested();
                } else {
                    // If this is the case, we need to fall back
                    error = "I couldn't work out how to open the requested document.";
                }
            } else {
                error = "An error occurred while trying to download the document.";
            }
        }

        if (!error.isEmpty()) {
            if (!fallbackLinks.isEmpty()) {
                QVariantMap link(fallbackLinks.takeFirst().toMap());
                QUrl url(QUrl::fromEncoded(link.value("url").toString().toUtf8()));
                bool validate(link.value(":validate", true).toBool());
                if (validate) {
                    QString expects(link.value("mime").toString());

                    setProgressMsg("Fetching...", url);

                    tab->setUrl(url);

                    QNetworkReply * reply = networkAccessManager()->get(QNetworkRequest(url));
                    reply->setProperty("__originalExpects", expects);
                    reply->setProperty("__originalParams", params);
                    reply->setProperty("__thisLink", link);
                    reply->setProperty("__fallbackLinks", fallbackLinks);
                    connect(reply, SIGNAL(finished()), this, SLOT(onNetworkReplyFinished()));
                    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
                } else {
                    // If no validation is needed, hand it over to the OS
                    QDesktopServices::openUrl(url);
                    emit closeRequested();
                }
            } else {
                setState(PapyroTab::DownloadingErrorState);
                setError(error);
            }
        }
    }

    void PapyroTabPrivate::onNetworkReplyDownloadProgress(qint64 bytes, qint64 total)
    {
        // Set progress
        if (total <= 0) {
            tab->setProgress(-1.0);
        } else {
            tab->setProgress(bytes / (qreal) total);
        }
    }

    void PapyroTabPrivate::onPagerPageClicked(int index)
    {
        documentView->showPage(index + 1);
    }

    void PapyroTabPrivate::onProgressLinksLabelLinkActivated(const QString & link)
    {
        if (link == "close") {
            emit closeRequested();
        } if (link == "retry") {
            tab->open(tab->url());
        }
    }

    void PapyroTabPrivate::onQuickSearchBarNext()
    {
        documentView->focusNextSpotlight();
        pager->hideSpotlights(false);
    }

    void PapyroTabPrivate::onQuickSearchBarPrevious()
    {
        documentView->focusPreviousSpotlight();
        pager->hideSpotlights(false);
    }

    void PapyroTabPrivate::onQuickSearchBarSearchForText(QString text)
    {
        if (text.trimmed().isEmpty()) {
            documentView->clearSearch();
            pager->setSpotlights();
        } else {
            Spine::TextExtentSet results;
            try {
                if (text.startsWith('/') && text.endsWith('/')) {
                    results = documentView->search(text.mid(1, text.size() - 2), Spine::RegExp | Spine::IgnoreCase);
                } else {
                    results = documentView->search(text, Spine::IgnoreCase);
                }

                QMap< int, int > pageCounts;
                foreach (Spine::TextExtentHandle extent, results)
                {
                    int pageFrom = extent->first.cursor()->page()->pageNumber();
                    int pageTo = extent->second.cursor()->page()->pageNumber();
                    for (int idx = pageFrom - 1; idx < pageTo; ++idx) {
                        if (!pageCounts.contains(idx)) {
                            pageCounts[idx] = 0;
                        }
                        ++pageCounts[idx];
                    }
                }
                pager->setSpotlights(pageCounts);
                quickSearchBar->searchReturned(results.size());
            } catch (Spine::TextExtent::regex_exception error) {
                // Failed to compile regular expression properly
                quickSearchBar->failed();
                documentView->clearSearch();
                pager->setSpotlights();
            }
        }
    }

    void PapyroTabPrivate::onRemoveAnnotation(Spine::AnnotationHandle annotation)
    {
        document()->removeAnnotation(annotation);
        document()->addAnnotation(annotation, document()->deletedItemsScratchId());
        publishChanges();
    }

    void PapyroTabPrivate::onSidebarSelectionChanged()
    {
        documentView->selectNone();
    }

    void PapyroTabPrivate::open(Spine::DocumentHandle document, const QVariantMap & params, Athenaeum::CitationHandle citation)
    {
        // Open the given document in this tab
        if (document) {
            documentProxy->setDocument(document);
            tab->setCitation(citation);

            // Set up UI
            actionTogglePager->setEnabled(true);
            documentView->setZoomMode(DocumentView::FitToWidth);
            documentView->setPageFlow(DocumentView::Continuous);
            documentView->setDocument(document);

            // Go to page/anchor/text according to params
            documentView->showPage(params);

            // Start the pager off generating thumbnails
            for (size_t i = 0; i < document->numberOfPages(); ++i) {
                pager->rename(pager->append(), QString("%1").arg(i+1));
                pagerQueue.append(i);
            }
            connect(&pagerTimer, SIGNAL(timeout()), this, SLOT(loadNextPagerImage()));
            pagerTimer.setInterval(0);
            pagerTimer.start();

            // Start the flowbrowser off generating images
            // Begin by finding all the bitmap bounding boxes
            Spine::AreaList areas;
            const Spine::Image * image = 0;
            for (Spine::CursorHandle cursor = document->newCursor();
                 (image = cursor->image()) || (image = cursor->nextImage(Spine::WithinDocument));
                 cursor->nextImage(Spine::WithinDocument)) {
                if (image->type() != Spine::Image::Null) {
                    Spine::Area area(Spine::Area(cursor->page()->pageNumber(), 0, image->boundingBox()));
                    if (area.boundingBox.x1 > area.boundingBox.x2) {
                        double tmp = area.boundingBox.x2;
                        area.boundingBox.x2 = area.boundingBox.x1;
                        area.boundingBox.x1 = tmp;
                    }
                    if (area.boundingBox.y1 > area.boundingBox.y2) {
                        double tmp = area.boundingBox.y2;
                        area.boundingBox.y2 = area.boundingBox.y1;
                        area.boundingBox.y1 = tmp;
                    }
                    areas.push_back(area);
                } else {
                     // FIXME NULL IMAGE RECEIVED
                }
            }
            // Then coalesce the bounding boxes and ignore tiny images
            areas = Spine::compile(areas);
            foreach (const Spine::Area & area, areas) {
                if ((area.boundingBox.width() * area.boundingBox.height()) > 5000.0 &&
                    area.boundingBox.width() > 50.0 && area.boundingBox.height() > 50.0) {
                    imageBrowserModel->append("");
                    imageAreas << area;
                }
            }

            // Set initial sidebar mode
            sidebar->setMode(Sidebar::DocumentWide);
            setState(PapyroTab::ProcessingState);

            loadAnnotators();

            // Make sure the incoming citation is stored in the document
            if (citation) {
                QVariantMap provenance(citation->field(Athenaeum::Citation::ProvenanceRole).toMap());
                QVariantList sources(provenance.value("sources").toList());
                if (sources.isEmpty()) {
                    document->addAnnotation(mapToCitation(citation->toMap()), "Document Metadata");
                } else {
                    foreach (const QVariant & source, sources) {
                        document->addAnnotation(mapToCitation(source.toMap()), "Document Metadata");
                    }
                }
            }

            // Populate document after a short pause
            QTimer::singleShot(500, this, SLOT(on_load_event_chain()));
        } else {
            // FIXME broken tab
            setState(PapyroTab::LoadingErrorState);
            setError("Error opening this document.");
        }
    }

    bool PapyroTabPrivate::on_load_event_chain()
    {
        bool queued = false;

        // Setting off the event handlers
        queued = handleEvent("load");
        queued = handleEvent("ready") || queued;
        queued = queued && handleEvent("filter");

        return queued;
    }

    void PapyroTabPrivate::publishChanges()
    {
        QEventLoop eventLoop;
        if (on_marshal_event_chain(&eventLoop, SLOT(quit()))) {
            eventLoop.exec();
        }
    }

    void PapyroTabPrivate::queueAnnotatorRunnable(AnnotatorRunnable * runnable)
    {
        ProgressLozenge * lozenge = 0;
#ifdef _WIN32
        char env[1024] = { 0 };
        int status = GetEnvironmentVariable("UTOPIA_LOZENGES", env, sizeof(env));
        if (status == 0) { env[0] = 0; }
        bool lozenges = (env[0] && strcmp(env, "on") == 0);
#else
        char * env = ::getenv("UTOPIA_LOZENGES");
        bool lozenges = (env && strcmp(env, "on") == 0);
#endif

        if (lozenges) {
            QColor color = QColor(30, 0, 0);

            // Create new status stack bar
            lozenge = new ProgressLozenge(runnable->title(), color);
            //        connect(&statusWidgetTimer, SIGNAL(timeout()), lozenge, SLOT(update()));
        }

        // Connect management signals
        connect(runnable, SIGNAL(started()), this, SLOT(onAnnotatorStarted()));
        connect(runnable, SIGNAL(finished()), this, SLOT(onAnnotatorFinished()));

        // Connect GUI signals
        if (lozenges) {
            connect(runnable, SIGNAL(started()), lozenge, SLOT(start()));
            connect(runnable, SIGNAL(finished()), lozenge, SLOT(deleteLater()));
        }
        connect(runnable, SIGNAL(finished()), documentView, SLOT(updateAnnotations()));

        // Queue runnable
        annotatorPool.start(runnable);
        if (lozenges) {
            statusLayout->insertWidget(1, new WidgetExpander(lozenge, tab), 0, statusAlignment);
        }
        ++activeAnnotators;
    }

    void PapyroTabPrivate::receiveFromBus(const QString & sender, const QVariant & data)
    {
        QVariantMap map(data.toMap());
        QUuid recipientUuid(map.value("uuid").toString());
        if (!recipientUuid.isNull()) {
            foreach (boost::shared_ptr< Annotator > annotator, annotators) {
                QUuid pluginUuid(annotator->configurationId().toString());
                if (pluginUuid == recipientUuid) {
                    setState(PapyroTab::ProcessingState);
                    QVariantMap sanitised;
                    if (!map.value("data").isNull()) {
                        sanitised["data"] = map.value("data");
                    }
                    on_activate_event_chain(annotator, sanitised);
                }
            }
        }
    }

    void PapyroTabPrivate::reloadAnnotators()
    {
        unloadAnnotators();
        loadAnnotators();
    }

    void PapyroTabPrivate::requestImage(int index)
    {
        pagerQueue.removeAll(index);
        pagerQueue.prepend(index);
    }

    void PapyroTabPrivate::resubscribeToBus()
    {
        subscribeToBus();
    }

    void PapyroTabPrivate::setError(const QString & reason)
    {
        static const QString infoTpl("<b style=\"color:red\"><big>Oops...</big><br>%1</b>");
        static const QString urlTpl("<small style=\"color:grey\">%1</small>");
        static const QString linksTpl("<a href=\"retry\" style=\"color:grey\">Retry</a> | <a href=\"close\" style=\"color:grey\">Cancel</a>");
        if (error != reason) {
            error = reason;
            progressInfoLabel->setText(infoTpl.arg(reason));
            progressUrlLabel->setVisible(url.isValid());
            progressUrlLabel->setText(urlTpl.arg(url.isLocalFile() ? "\"" + url.toLocalFile() + "\"" : url.toString()));
            progressLinksLabel->setText(linksTpl);
            emit errorChanged(error);
        }
    }

    void PapyroTabPrivate::setProgressMsg(const QString & msg, const QUrl & url)
    {
        static const QString infoTpl("<span>%1</span>");
        static const QString urlTpl("<small style=\"color:grey\">%1</small>");
        progressInfoLabel->setText(infoTpl.arg(msg));
        progressUrlLabel->show();
        progressUrlLabel->setText(urlTpl.arg(url.toString()));
        progressLinksLabel->setText(QString());
    }

    void PapyroTabPrivate::setState(PapyroTab::State newState)
    {
        if (state != newState) {
            // Reset label message
            if (state == PapyroTab::DownloadingErrorState ||
                state == PapyroTab::LoadingErrorState) {
                setError(QString());
            }

            state = newState;

            progressIconLabel->hide();
            progressInfoLabel->setText(QString());
            progressUrlLabel->setText(QString());
            progressLinksLabel->setText(QString());

            progressSpinner->stop();
            progressSpinner->show();
            progressSpinner->setProgress(-1.0);

            switch (state) {
            case PapyroTab::UninitialisedState:
            case PapyroTab::EmptyState:
            case PapyroTab::LoadingState:
                progressWidget->show();
                break;
            case PapyroTab::DownloadingState:
                progressWidget->show();
                progressSpinner->start();
                break;
            case PapyroTab::DownloadingErrorState:
            case PapyroTab::LoadingErrorState:
                progressWidget->show();
                progressSpinner->hide();
                progressIconLabel->show();
                break;
            case PapyroTab::ProcessingState:
            case PapyroTab::IdleState:
                progressWidget->hide();
                break;
            }

            if (state == PapyroTab::IdleState) {
                // Document is ready
                onDocumentProcessingFinished();
            }

            emit stateChanged(state);

            tab->update();
        }
    }

    void PapyroTabPrivate::showPager(bool show)
    {
        pager->setVisible(show);
    }

    void PapyroTabPrivate::showImageBrowser(bool show)
    {
        imageBrowser->setMinimumHeight(80);
        imageBrowser->setVisible(show);
    }

    void PapyroTabPrivate::showSidebar(bool show)
    {
        sidebar->setVisible(show);
    }

    void PapyroTabPrivate::showLookupBar(bool show)
    {
        lookupWidget->setVisible(show);
        lookupWidget->raise();
    }

    void PapyroTabPrivate::unloadAnnotators()
    {
        if (ready) {
            // Cleanup annotators FIXME make sure this is done at correct times
            handleEvent("close");

            annotators.clear();
            activatableAnnotators.clear();
            lookups.clear();
            eventHandlers.clear();

            ready = false;
        }
    }

    void PapyroTabPrivate::visualiseAnnotations(Spine::AnnotationSet annotations, const QVariantMap & context)
    {
        QStringList terms;
        Spine::AnnotationSet ignore;
        size_t page = context.value("page", 0).toUInt();
        QPointF pagePos = context.value("pos", 0).toPointF();
        foreach (Spine::AnnotationHandle annotation, annotations) {
            // Ignore items that are supposed to be embedded
            if (annotation->getFirstProperty("property:embedded") == "1" ||
                annotation->getFirstProperty("property:demo_logo") == "1") {
                ignore.insert(annotation);
            }
        }
        foreach (Spine::AnnotationHandle annotation, ignore) {
            annotations.erase(annotation);
        }

        // Compile contexts for each annotation
        QList< QPair< Spine::AnnotationHandle, QVariantMap > > contextualised;
        foreach (Spine::AnnotationHandle annotation, annotations) {
            QVariantMap annotationContext(context);
            // Compile a list of searchable terms
            foreach (Spine::TextExtentHandle extent, annotation->extents()) {
                QVariantMap extentContext;
                foreach (Spine::Area area, extent->areas()) {
                    if (area.page == page && area.boundingBox.contains(pagePos.x(), pagePos.y())) {
                        QVariantList bounds;
                        foreach (Spine::Area area, extent->areas()) {
                            QVariantList bound;
                            bound << area.page;
                            bound << area.boundingBox.x1 << area.boundingBox.y1 << area.boundingBox.x2 << area.boundingBox.y2;
                            bounds << bound;
                        }
                        extentContext["text"] = qStringFromUnicode(extent->text());
                        extentContext["bounds"] = bounds;
                        annotationContext["extent"] = extentContext;
                        annotationContext["term"] = extentContext["text"].toString().toLower();
                        terms.push_back(annotationContext["term"].toString());
                        break;
                    }
                }
            }
            contextualised << qMakePair(annotation, annotationContext);
        }
        terms.removeDuplicates();

        if (!contextualised.empty()) {
            actionToggleSidebar->setChecked(true);
            sidebar->setMode(Sidebar::Results);
            sidebar->resultsView()->clear();
            if (!terms.isEmpty()) {
                sidebar->resultsView()->setExploreTerms(terms, true);
            }
            typedef QPair< Spine::AnnotationHandle, QVariantMap > Pair;
            foreach (Pair pair, contextualised) {
                if (pair.first->capable< SummaryCapability >()) {
                    QPointF pagePos = pair.second["pos"].toPointF();
                    QVariantList pagePosList;
                    pagePosList << pagePos.x() << pagePos.y();
                    pair.second["pos"] = pagePosList;
                    //qDebug() << "addResult" << pair.second;
                    sidebar->resultsView()->addResult(new AnnotationResultItem(pair.first, pair.second));
                }
            }
        }
    }





    /// PapyroTab ///////////////////////////////////////////////////////////////////////

    PapyroTab::PapyroTab(QWidget * parent)
        : QFrame(parent), d(new PapyroTabPrivate(this))
    {
        connect(d, SIGNAL(contextMenuAboutToShow(QMenu *)), this, SIGNAL(contextMenuAboutToShow(QMenu *)));
        connect(d, SIGNAL(errorChanged(const QString &)), this, SIGNAL(errorChanged(const QString &)));
        connect(d, SIGNAL(stateChanged(PapyroTab::State)), this, SIGNAL(stateChanged(PapyroTab::State)));
        connect(d, SIGNAL(starredChanged(bool)), this, SIGNAL(starredChanged(bool)));
        connect(d, SIGNAL(knownChanged(bool)), this, SIGNAL(knownChanged(bool)));
        connect(d, SIGNAL(closeRequested()), this, SIGNAL(closeRequested()));

        // Register signal handlers for managing annotations
        d->documentProxy = new DocumentProxy(this);
        connect(d->documentProxy, SIGNAL(areaSelectionChanged(const std::string &, const Spine::AreaSet &, bool)),
                d, SLOT(onDocumentAreaSelectionChanged(const std::string &, const Spine::AreaSet &, bool)));
        connect(d->documentProxy, SIGNAL(textSelectionChanged(const std::string &, const Spine::TextExtentSet &, bool)),
                d, SLOT(onDocumentTextSelectionChanged(const std::string &, const Spine::TextExtentSet &, bool)));
        connect(d->documentProxy, SIGNAL(annotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)),
                d, SLOT(onDocumentAnnotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)));

        // Main horizontal layout for this tab (document / sidebar)
        d->mainLayout = new QStackedLayout(this);
        d->mainLayout->setSpacing(0);
        d->mainLayout->setContentsMargins(0, 0, 0, 0);
        d->mainLayout->setStackingMode(QStackedLayout::StackAll);

        d->watermarkRenderer.load(QString(":/images/utopia-spiral-black.svg"));

        // Loading tab page
        d->progressWidget = new QFrame;
        d->progressWidget->installEventFilter(d);
        d->progressWidget->setObjectName("progress_widget");
        QVBoxLayout * progressLayout = new QVBoxLayout(d->progressWidget);
        progressLayout->addStretch(1);
        progressLayout->addWidget(d->progressIconLabel = new QLabel, 0, Qt::AlignCenter);
        progressLayout->addWidget(d->progressSpinner = new Utopia::Spinner, 0, Qt::AlignCenter);
        progressLayout->addWidget(d->progressInfoLabel = new QLabel, 0, Qt::AlignCenter);
        progressLayout->addWidget(d->progressUrlLabel = new QLabel, 0, Qt::AlignCenter);
        progressLayout->addWidget(d->progressLinksLabel = new QLabel, 0, Qt::AlignCenter);
        progressLayout->addStretch(1);
        d->progressIconLabel->setFixedSize(QSize(128, 128));
        d->progressIconLabel->setAlignment(Qt::AlignCenter);
        d->progressIconLabel->setPixmap(QPixmap(":/icons/broken-tab.png"));
        d->progressIconLabel->hide();
        d->progressUrlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        d->progressUrlLabel->setCursor(Qt::IBeamCursor);
        connect(d->progressLinksLabel, SIGNAL(linkActivated(const QString &)), d, SLOT(onProgressLinksLabelLinkActivated(const QString &)));
        d->progressSpinner->setFixedSize(QSize(64, 64));
        d->progressInfoLabel->setAlignment(Qt::AlignCenter);
        d->progressUrlLabel->setAlignment(Qt::AlignCenter);
        d->progressLinksLabel->setAlignment(Qt::AlignCenter);
        connect(d->progressSpinner, SIGNAL(progressChanged(qreal)), this, SIGNAL(progressChanged(qreal)));

        // Splitter between the document and the sidebar
        // FIXME perhaps this should just be a styled widget in the horizontal layout,
        // given the sidebar isn't resizable
        d->contentSplitter = new QSplitter(Qt::Horizontal);
        d->contentSplitter->setChildrenCollapsible(false);

        QWidget * pagerLayoutWidget = new QWidget;
        pagerLayoutWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        QVBoxLayout * pagerLayout = new QVBoxLayout(pagerLayoutWidget);
        pagerLayout->setContentsMargins(0, 0, 0, 0);
        pagerLayout->setSpacing(0);
        d->contentSplitter->addWidget(pagerLayoutWidget);
        d->contentSplitter->setHandleWidth(1);

        // Document View
        {
            QSplitter * documentSplitter = new QSplitter(Qt::Vertical);
            documentSplitter->setChildrenCollapsible(false);
            documentSplitter->setHandleWidth(1);

            // Flow Browser
            {
                d->imageBrowser = new Utopia::FlowBrowser;
                d->imageBrowser->setDefaultBackgroundColor(QColor(245, 245, 255));
                d->imageBrowserModel = d->imageBrowser->addModel("Figures");
                d->chemicalBrowserModel = d->imageBrowser->addModel("Chemicals");
                d->imageBrowser->setMaximumHeight(300);
                documentSplitter->addWidget(d->imageBrowser);
                d->imageBrowser->resize(0, 0);
                d->imageBrowser->hide();

                connect(d->imageBrowserModel, SIGNAL(requiresUpdate(int)), d, SLOT(loadImage(int)));
                connect(d->imageBrowserModel, SIGNAL(selected(int)), d, SLOT(activateImage(int)));
                connect(d->imageBrowserModel, SIGNAL(emptinessChanged(bool)), d, SLOT(onImageBrowserEmptinessChanged(bool)));

                connect(d->chemicalBrowserModel, SIGNAL(requiresUpdate(int)), d, SLOT(loadChemicalImage(int)));
                connect(d->chemicalBrowserModel, SIGNAL(selected(int)), d, SLOT(activateChemicalImage(int)));
            }

            // Main document view
            d->documentView = new DocumentView();
            //d->documentView->setPageDecorations(DocumentView::Shadows);
            d->documentView->setZoomMode(DocumentView::FitToWindow);
            d->documentView->setMinimumSize(200, 300);
            d->documentView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
//            d->documentView->setContentsMargins(0, 0, 0, 0);
//            d->documentView->setSpacing(1);
//            d->documentView->setSpineSpacing(0);
            d->documentView->installEventFilter(d);
            documentSplitter->addWidget(d->documentView);
            pagerLayout->addWidget(documentSplitter);

            QSplitterHandle * contentSplitterHandle = documentSplitter->handle(1);
            contentSplitterHandle->installEventFilter(d);
            contentSplitterHandle->setAutoFillBackground(true);

            connect(d->documentView, SIGNAL(contextMenuAboutToShow(QMenu*, Spine::DocumentHandle, Spine::CursorHandle)),
                    d, SLOT(onDocumentViewContextMenu(QMenu*, Spine::DocumentHandle, Spine::CursorHandle)));
            connect(d->documentView, SIGNAL(focusChanged(PageView*,QPointF)),
                    d, SLOT(focusChanged(PageView*,QPointF)));
            connect(d->documentView, SIGNAL(annotationsActivated(Spine::AnnotationSet, const QVariantMap &)),
                    d, SLOT(onDocumentViewAnnotationsActivated(Spine::AnnotationSet, const QVariantMap &)));
            connect(d->documentView, SIGNAL(pageFocusChanged(size_t)),
                    d, SLOT(onDocumentViewPageFocusChanged(size_t)));
            //connect(d->documentView, SIGNAL(selectionManaged(Spine::TextSelection,bool)),
            //        d, SLOT(onDocumentViewManageSelection(Spine::TextSelection,bool)));
            //connect(d->documentView, SIGNAL(selectionManaged(Spine::AreaSet)),
            //        d, SLOT(onDocumentViewManageSelection(Spine::AreaSet)));
            connect(d->documentView, SIGNAL(exploreSelection()),
                    d, SLOT(exploreSelection()));
            connect(d->documentView, SIGNAL(publishChanges()),
                    d, SLOT(publishChanges()));
            connect(d->documentView, SIGNAL(urlRequested(const QUrl &, const QString &)),
                    this, SIGNAL(urlRequested(const QUrl &, const QString &)));
        }

        // Quick search bar
        {
            // Quick search bar
            d->quickSearchBar = new SearchBar(this);
			d->quickSearchBar->setFixedHeight(d->quickSearchBar->height() * Utopia::hiDPIScaling());
            d->quickSearchBar->hide();

            connect(d->quickSearchBar, SIGNAL(searchForText(QString)), d, SLOT(onQuickSearchBarSearchForText(QString)));
            connect(d->quickSearchBar, SIGNAL(previous()), d, SLOT(onQuickSearchBarPrevious()));
            connect(d->quickSearchBar, SIGNAL(next()), d, SLOT(onQuickSearchBarNext()));
        }

        // Sidebar
        {
            d->sidebar = new Sidebar;
            d->sidebar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
            d->sidebar->setDocumentProxy(d->documentProxy);
            d->contentSplitter->addWidget(d->sidebar);


            connect(d->sidebar, SIGNAL(selectionChanged()), d, SLOT(onSidebarSelectionChanged()));
            connect(d->sidebar, SIGNAL(urlRequested(const QUrl &, const QString &)),
                    this, SIGNAL(urlRequested(const QUrl &, const QString &)));
            connect(d->sidebar, SIGNAL(citationsActivated(const QVariantList &, const QString &)),
                    this, SIGNAL(citationsActivated(const QVariantList &, const QString &)));
            connect(d->sidebar, SIGNAL(termExplored(const QString &)),
                    d, SLOT(explore(const QString &)));
        }

        // Manual lookup bar
        {
            d->lookupWidget = new QFrame(this);
            d->lookupWidget->setObjectName("lookupBar");
            d->lookupWidget->setFixedWidth(d->sidebar->width());
            d->lookupWidget->installEventFilter(d);
            d->lookupWidget->hide();
            QHBoxLayout * hLayout = new QHBoxLayout(d->lookupWidget);
            d->lookupTextBox = new QLineEdit(this);
            d->lookupTextBox->setAttribute(Qt::WA_MacShowFocusRect, 0);

            hLayout->addWidget(d->lookupTextBox);
            d->lookupButton = new QPushButton("Explore");
            connect(d->lookupButton, SIGNAL(clicked()), d, SLOT(onLookupOverride()));
            connect(d->lookupTextBox, SIGNAL(returnPressed()), d, SLOT(onLookupOverride()));
            hLayout->addWidget(d->lookupButton);
            hLayout->setSpacing(4);
            hLayout->setContentsMargins(0, 0, 0, 0);
            d->lookupWidget->adjustSize();
            d->lookupWidget->hide();
        }

        QSplitterHandle * contentSplitterHandle = d->contentSplitter->handle(1);
        contentSplitterHandle->installEventFilter(d);
        contentSplitterHandle->setAutoFillBackground(false);
        if (d->sidebar->minimumWidth() == d->sidebar->maximumWidth()) {
            contentSplitterHandle->setCursor(Qt::ArrowCursor);
        } else {
			QList< int > sizes;
			sizes << QApplication::desktop()->availableGeometry().width();
			sizes << d->sidebar->minimumWidth();
			d->contentSplitter->setSizes(sizes);
		}

        // Pager
        {
            d->pager = new Pager();
            d->pager->setAutoFillBackground(true);
            d->pager->setSpread(0.1);
            d->pager->setMinimumHeight(80);
            d->pager->setMaximumHeight(120);
            d->pager->setContentsMargins(10, 10, 10, 11);
            d->pager->setDrawLabels(false);
            d->pager->hide();
            pagerLayout->addWidget(d->pager);

            connect(d->pager, SIGNAL(pageClicked(int)),
                    d, SLOT(onPagerPageClicked(int)));
            connect(d->documentView, SIGNAL(spotlightsHidden()),
                    d->pager, SLOT(hideSpotlights()));
            connect(d->documentView, SIGNAL(spotlightsHidden()),
                    d, SLOT(onDocumentViewSpotlightsHidden()));
            connect(d->quickSearchBar, SIGNAL(clearSearch()),
                    d->documentView, SLOT(clearSearch()));
            connect(d->quickSearchBar, SIGNAL(clearSearch()),
                    d->pager, SLOT(setSpotlights()));
        }

        // Set up dispatcher
        d->dispatcher = new Dispatcher(this);
        d->dispatcher->setDecorators(d->decorators);

        // Connect up dispatcher
        connect(d->dispatcher, SIGNAL(started()),
                d, SLOT(onLookupStarted()));
        connect(d->dispatcher, SIGNAL(started()),
                d->sidebar, SLOT(lookupStarted()));

        connect(d->dispatcher, SIGNAL(cleared()),
                d, SLOT(onLookupStopped()));
        connect(d->dispatcher, SIGNAL(finished()),
                d, SLOT(onLookupStopped()));
        connect(d->dispatcher, SIGNAL(finished()),
                d->sidebar, SLOT(lookupStopped()));

        //connect(dispatcher, SIGNAL(started()),
        //        spinner, SLOT(start()));
        //connect(dispatcher, SIGNAL(cleared()),
        //        spinner, SLOT(stop()));
        //connect(dispatcher, SIGNAL(finished()),
        //        spinner, SLOT(stop()));
        //connect(dispatcher, SIGNAL(progressed(qreal)),
        //        spinner, SLOT(setProgress(qreal)));

        qRegisterMetaType< Spine::AnnotationHandle >("Spine::AnnotationHandle");
        connect(d->dispatcher, SIGNAL(annotationFound(Spine::AnnotationHandle)),
                d, SLOT(onDispatcherAnnotationFound(Spine::AnnotationHandle)), Qt::QueuedConnection);

        // Tab actions
        d->actionQuickSearch = new QAction("Find In Document...", this);
        {
        QList< QKeySequence > shortcuts;
        shortcuts << QKeySequence::Find;
        shortcuts << QKeySequence(Qt::Key_Slash);
        d->actionQuickSearch->setShortcuts(shortcuts);
        }
        QObject::connect(d->actionQuickSearch, SIGNAL(triggered()), this, SLOT(quickSearch()));
        d->actions[QuickSearch] = d->actionQuickSearch;

        d->actionQuickSearchNext = new QAction("Find Next", this);
        d->actionQuickSearchNext->setShortcut(QKeySequence::FindNext);
        QObject::connect(d->actionQuickSearchNext, SIGNAL(triggered()), this, SLOT(quickSearchNext()));
        d->actions[QuickSearchNext] = d->actionQuickSearchNext;

        d->actionQuickSearchPrevious = new QAction("Find Previous", this);
        d->actionQuickSearchPrevious->setShortcut(QKeySequence::FindPrevious);
        QObject::connect(d->actionQuickSearchPrevious, SIGNAL(triggered()), this, SLOT(quickSearchPrevious()));
        d->actions[QuickSearchPrevious] = d->actionQuickSearchPrevious;

        QIcon iconTogglePager;
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-active.png"), QIcon::Selected, QIcon::On);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-active.png"), QIcon::Selected, QIcon::Off);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-active.png"), QIcon::Active, QIcon::On);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-active.png"), QIcon::Active, QIcon::Off);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-on.png"), QIcon::Normal, QIcon::On);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-off.png"), QIcon::Normal, QIcon::Off);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-disabled.png"), QIcon::Disabled, QIcon::Off);
        iconTogglePager.addPixmap(QPixmap(":/icons/pages-disabled.png"), QIcon::Disabled, QIcon::On);
        d->actionTogglePager = new QAction(iconTogglePager, "Toggle Pager", this);
        d->actionTogglePager->setShortcut(QKeySequence(Qt::ALT + Qt::Key_P));
        d->actionTogglePager->setCheckable(true);
        d->actionTogglePager->setChecked(false);
        d->actionTogglePager->setDisabled(true);
        QObject::connect(d->actionTogglePager, SIGNAL(toggled(bool)), d, SLOT(showPager(bool)));
        //addAction(d->actionTogglePager);
        d->actions[TogglePager] = d->actionTogglePager;

        QIcon iconToggleImageBrowser;
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-active.png"), QIcon::Selected, QIcon::On);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-active.png"), QIcon::Selected, QIcon::Off);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-active.png"), QIcon::Active, QIcon::On);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-active.png"), QIcon::Active, QIcon::Off);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-on.png"), QIcon::Normal, QIcon::On);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-off.png"), QIcon::Normal, QIcon::Off);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-disabled.png"), QIcon::Disabled, QIcon::Off);
        iconToggleImageBrowser.addPixmap(QPixmap(":/icons/flow-disabled.png"), QIcon::Disabled, QIcon::On);
        d->actionToggleImageBrowser = new QAction(iconToggleImageBrowser, "Toggle Figure Browser", this);
        d->actionToggleImageBrowser->setShortcut(QKeySequence(Qt::ALT + Qt::Key_I));
        d->actionToggleImageBrowser->setCheckable(true);
        d->actionToggleImageBrowser->setChecked(false);
        d->actionToggleImageBrowser->setDisabled(true);
        QObject::connect(d->actionToggleImageBrowser, SIGNAL(toggled(bool)), d, SLOT(showImageBrowser(bool)));
        //addAction(d->actionToggleImageBrowser);
        d->actions[ToggleImageBrowser] = d->actionToggleImageBrowser;

        QIcon iconToggleSidebar;
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-active.png"), QIcon::Selected, QIcon::On);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-active.png"), QIcon::Selected, QIcon::Off);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-active.png"), QIcon::Active, QIcon::On);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-active.png"), QIcon::Active, QIcon::Off);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-on.png"), QIcon::Normal, QIcon::On);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-off.png"), QIcon::Normal, QIcon::Off);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-disabled.png"), QIcon::Disabled, QIcon::Off);
        iconToggleSidebar.addPixmap(QPixmap(":/icons/eye-disabled.png"), QIcon::Disabled, QIcon::On);
        d->actionToggleSidebar = new QAction(iconToggleSidebar, "Toggle Sidebar", this);
        d->actionToggleSidebar->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
        d->actionToggleSidebar->setCheckable(true);
        d->actionToggleSidebar->setChecked(true);
        QObject::connect(d->actionToggleSidebar, SIGNAL(toggled(bool)), d, SLOT(showSidebar(bool)));
        //addAction(d->actionToggleSidebar);
        d->actions[ToggleSidebar] = d->actionToggleSidebar;

        QIcon iconToggleLookupBar;
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-active.png"), QIcon::Selected, QIcon::On);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-active.png"), QIcon::Selected, QIcon::Off);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-active.png"), QIcon::Active, QIcon::On);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-active.png"), QIcon::Active, QIcon::Off);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-on.png"), QIcon::Normal, QIcon::On);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-off.png"), QIcon::Normal, QIcon::Off);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-disabled.png"), QIcon::Disabled, QIcon::Off);
        iconToggleLookupBar.addPixmap(QPixmap(":/icons/magnifyingglass-disabled.png"), QIcon::Disabled, QIcon::On);
        d->actionToggleLookupBar = new QAction(iconToggleLookupBar, "Toggle Lookup Search Box", this);
        {
        QList< QKeySequence > shortcuts;
        shortcuts << QKeySequence("Alt+"+QKeySequence(QKeySequence::Find).toString());
        shortcuts << QKeySequence(Qt::ALT + Qt::Key_Slash);
        d->actionToggleLookupBar->setShortcuts(shortcuts);
        }
        d->actionToggleLookupBar->setCheckable(true);
        d->actionToggleLookupBar->setChecked(false);
        //QObject::connect(d->actionToggleLookupBar, SIGNAL(toggled(bool)), d, SLOT(showLookupBar(bool)));
        //addAction(d->actionToggleLookupBar);
        d->actions[ToggleLookupBar] = d->actionToggleLookupBar;

        {
            QAction * action = 0;
            QList< QKeySequence > shortcuts;

            action = new QAction("Next Page", this);
            shortcuts.clear();
            shortcuts << QKeySequence(Qt::Key_Down);
            shortcuts << QKeySequence(Qt::Key_PageDown);
            shortcuts << QKeySequence(Qt::Key_Right);
            action->setShortcuts(shortcuts);
            action->setShortcutContext(Qt::WindowShortcut);
            QObject::connect(action, SIGNAL(triggered()), d->documentView, SLOT(showNextPage()));
            //addAction(action);

            action = new QAction("Previous Page", this);
            shortcuts.clear();
            shortcuts << QKeySequence(Qt::Key_Up);
            shortcuts << QKeySequence(Qt::Key_PageUp);
            shortcuts << QKeySequence(Qt::Key_Left);
            action->setShortcuts(shortcuts);
            action->setShortcutContext(Qt::WindowShortcut);
            QObject::connect(action, SIGNAL(triggered()), d->documentView, SLOT(showPreviousPage()));
            //addAction(action);

            action = new QAction("Last Page", this);
            shortcuts.clear();
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_Down);
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_PageDown);
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_Right);
            shortcuts << QKeySequence(Qt::Key_End);
            action->setShortcuts(shortcuts);
            action->setShortcutContext(Qt::WindowShortcut);
            QObject::connect(action, SIGNAL(triggered()), d->documentView, SLOT(showLastPage()));
            //addAction(action);

            action = new QAction("First Page", this);
            shortcuts.clear();
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_Up);
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_PageUp);
            shortcuts << QKeySequence(Qt::CTRL + Qt::Key_Left);
            shortcuts << QKeySequence(Qt::Key_Home);
            action->setShortcuts(shortcuts);
            action->setShortcutContext(Qt::WindowShortcut);
            QObject::connect(action, SIGNAL(triggered()), d->documentView, SLOT(showFirstPage()));
            //addAction(action);
        }

        d->mainLayout->addWidget(d->progressWidget);
        d->mainLayout->addWidget(d->contentSplitter);

        d->statusWidgetTimer.setInterval(0);
//        connect(&d->statusWidgetTimer, SIGNAL(timeout()), d->documentView->viewport(), SLOT(update()));
        d->statusLayout = new QVBoxLayout(d->documentView->viewport());
        d->statusLayout->setContentsMargins(0, 0, 0, 0);
        d->statusLayout->setSpacing(0);
        d->statusLayout->addStretch(1);
        d->statusAlignment = Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter);
        d->activeAnnotators = 0;

        // Signal mappers
        d->annotatorMapper = new QSignalMapper(this);
        QObject::connect(d->annotatorMapper, SIGNAL(mapped(int)), d, SLOT(executeAnnotator(int)));

        // Registering annotation / selection processors
        foreach (AnnotationProcessor * processor, Utopia::instantiateAllExtensions< AnnotationProcessor >()) {
            d->annotationProcessors << processor;
        }
        foreach (SelectionProcessorFactory * processorFactory, Utopia::instantiateAllExtensions< SelectionProcessorFactory >()) {
            d->selectionProcessorFactories << processorFactory;
        }

        d->loadAnnotators();

        d->setState(EmptyState);
    }

    PapyroTab::~PapyroTab()
    {
        // Clear pool
        d->cancelRunnables();

        // Delete extensions
        d->annotators.clear();
        d->activatableAnnotators.clear();
        d->eventHandlers.clear();
        d->lookups.clear();
        d->annotationProcessors.clear();
        d->selectionProcessorFactories.clear();

        // Clean up children that rely on PapyroTab being there
        delete d->pager;
        delete d->imageBrowser;
        delete d->documentView;
        delete d->dispatcher;
    }

    void PapyroTab::addToLibrary()
    {
        if (d->citation && !d->citation->isKnown()) {
            Athenaeum::Bibliography * master = d->libraryModel->master();
            master->appendItem(d->citation);
            d->citation->setField(Athenaeum::Citation::DateImportedRole, QDateTime::currentDateTime());
            d->citation->setField(Athenaeum::Citation::OriginatingUriRole, url());
            std::string str(d->document()->data());
            QByteArray data(str.c_str(), str.size());
            d->libraryModel->saveObjectFile(d->citation, data, ".pdf");
        }
    }

    QAction * PapyroTab::action(ActionType actionType) const
    {
        return d->actions.value(actionType, 0);
    }

    SelectionProcessorAction * PapyroTab::activeSelectionProcessorAction() const
    {
        return d->activeSelectionProcessorAction;
    }

    Utopia::Bus * PapyroTab::bus() const
    {
        return d->bus();
    }

    Athenaeum::CitationHandle PapyroTab::citation() const
    {
        return d->citation;
    }

    void PapyroTab::clear()
    {
        d->cancelRunnables();

        // Clear pager
        d->pager->clear();
        d->actionTogglePager->setChecked(false);
        d->actionTogglePager->setEnabled(false);

        // Clear flow browser
        d->imageBrowserModel->clear();
        d->chemicalBrowserModel->clear();
        d->imageBrowser->setCurrentModel(d->imageBrowserModel);
        d->imageAreas.clear();
        d->chemicalExtents.clear();
        d->actionToggleImageBrowser->setChecked(false);

        // Clear and hide sidebar
        d->sidebar->clear();

        d->documentProxy->setDocument(Spine::DocumentHandle());

        // Clear documentView
        d->documentView->clear();
        d->documentView->setZoomMode(DocumentView::FitToWindow);

        d->citation.reset();

        setUrl(QUrl());
        setTitle("");
        setProgress(-1.0);

        d->setState(EmptyState);

        d->unloadAnnotators();

        emit documentChanged();
    }

    void PapyroTab::clearActiveSelectionProcessorAction()
    {
        setActiveSelectionProcessorAction();
    }

    void PapyroTab::closeEvent(QCloseEvent * event)
    {
        // FIXME check if something needs to be asked of the user before closing
        bool ok = true;

        if (ok) {
            // clear this tab
            clear();
            // accept close
            event->accept();
        }
    }

    void PapyroTab::copySelectedText()
    {
        // Error tabs...
        if (d->progressUrlLabel->isVisible()) {
            QClipboard * clipboard = QApplication::clipboard();
            clipboard->setText(d->progressUrlLabel->selectedText(), QClipboard::Clipboard);
        } else {
        // Non-error tabs...
            Spine::DocumentHandle document(d->document());
            QString documentSelection = document ? qStringFromUnicode(document->textSelection().text()) : QString();
            if (!documentSelection.isEmpty()) {
                d->documentView->copySelectedText();
            } else {
                d->sidebar->copySelectedText();
            }
        }
    }

    Spine::DocumentHandle PapyroTab::document()
    {
        return d->document();
    }

    DocumentView * PapyroTab::documentView() const
    {
        return d->documentView;
    }

    QString PapyroTab::error() const
    {
        return d->error;
    }

    void PapyroTab::exploreSelection()
    {
        d->exploreSelection();
    }

    bool PapyroTab::isEmpty() const
    {
        return state() == EmptyState || state() == LoadingErrorState || state() == DownloadingErrorState;
    }

    bool PapyroTab::isKnown() const
    {
        return d->citation && d->citation->field(Athenaeum::Citation::KnownRole).toBool();
    }

    bool PapyroTab::isStarred() const
    {
        return d->citation && (d->citation->field(Athenaeum::Citation::FlagsRole).value< Athenaeum::Citation::Flags >() & Athenaeum::Citation::StarredFlag);
    }

    QNetworkAccessManager * PapyroTab::networkAccessManager() const
    {
        return d->networkAccessManager().get();
    }

    void PapyroTab::open(Spine::DocumentHandle document, const QVariantMap & params, Athenaeum::CitationHandle citation)
    {
        // Clear any previous content
        if (this->document()) {
            clear();
        }

        d->setState(LoadingState);
        setTitle(QString("Loading..."));

        d->open(document, params, citation);

        // Let other components know the document has changed
        if (document) {
            emit documentChanged();
        }
    }

    void PapyroTab::open(QIODevice * io, const QVariantMap & params, Athenaeum::CitationHandle citation)
    {
        // Clear any previous content
        if (document()) {
            clear();
        }

        d->setState(LoadingState);
        setTitle(QString("Loading..."));

        Spine::DocumentHandle document = d->documentManager->open(io);
        d->open(document, params, citation);

        // Let other components know the document has changed
        if (document) {
            emit documentChanged();
        }
    }

    void PapyroTab::open(const QString & filename, const QVariantMap & params, Athenaeum::CitationHandle citation)
    {
        // Clear any previous content
        if (document()) {
            clear();
        }

        d->setState(LoadingState);
        setTitle(QString("Loading..."));
        setUrl(QUrl::fromLocalFile(filename));

        Spine::DocumentHandle document = d->documentManager->open(filename);
        d->open(document, params, citation);

        // Let other components know the document has changed
        if (document) {
            emit documentChanged();
        }
    }

    void PapyroTab::open(const QUrl & url, const QVariantMap & params, Athenaeum::CitationHandle citation)
    {
        // Easier to go via files, rather than URLs
        if (url.scheme() == "file") {
            open(url.toLocalFile(), params);
            return;
        }

        // Clear any previous content
        if (document()) {
            clear();
        }

        QUrl realUrl(url);
        if (realUrl.scheme() == "utopiafile") {
            realUrl.setScheme("file");
        } else if (realUrl.scheme().startsWith("utopia")) {
            realUrl.setScheme(realUrl.scheme().replace("utopia", "http"));
        }

        // Cope properly with utopia(s): URLs
        d->setState(DownloadingState);
        d->setProgressMsg("Downloading...", realUrl);
        setUrl(realUrl);

        QNetworkReply * reply = d->networkAccessManager()->get(QNetworkRequest(realUrl));
        reply->setProperty("__originalCitation", QVariant::fromValue< Athenaeum::CitationHandle >(citation));
        reply->setProperty("__originalParams", params);
        connect(reply, SIGNAL(finished()), d, SLOT(onNetworkReplyFinished()));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), d, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
    }

    static bool link_less_than(const QVariant & lhs_, const QVariant & rhs_)
    {
        QVariantMap lhs(lhs_.toMap());
        QVariantMap rhs(rhs_.toMap());

        // Reverse order of mime type importance
        static QStringList mime_types_order;
        if (mime_types_order.isEmpty()) {
            mime_types_order << "text/html" << "application/pdf";
        }

        // Reverse order of link type importance
        static QStringList types_order;
        if (types_order.isEmpty()) {
            types_order << "search" << "abstract" << "article";
        }

        QString mime_lhs = lhs.value("mime").toString();
        QString mime_rhs = rhs.value("mime").toString();
        int order_mime_lhs = mime_types_order.contains(mime_lhs) ? mime_types_order.indexOf(mime_lhs) : -1;
        int order_mime_rhs = mime_types_order.contains(mime_rhs) ? mime_types_order.indexOf(mime_rhs) : -1;
        if (order_mime_lhs == order_mime_rhs) {
            QString type_lhs = lhs.value("type").toString();
            QString type_rhs = rhs.value("type").toString();
            int order_type_lhs = types_order.contains(type_lhs) ? types_order.indexOf(type_lhs) : -1;
            int order_type_rhs = types_order.contains(type_rhs) ? types_order.indexOf(type_rhs) : -1;
            if (order_type_lhs == order_type_rhs) {
                int weight_lhs = lhs.value(":weight").toInt();
                int weight_rhs = rhs.value(":weight").toInt();
                return weight_lhs > weight_rhs;
            } else {
                return order_type_lhs > order_type_rhs;
            }
        } else {
            return order_mime_lhs > order_mime_rhs;
        }
    }

/*
    void PapyroTab::open(const QVariantMap & citation, const QVariantMap & params)
    {
        // Clear any previous content
        if (document()) {
            clear();
        }

    }
 */

    void PapyroTab::open(Athenaeum::CitationHandle citation, const QVariantMap & params)
    {
        // Clear any previous content
        if (document()) {
            clear();
        }

        // Get the citation's links
        QList< QVariant > links;
        foreach (const QVariant & variant, citation->field(Athenaeum::Citation::LinksRole).toList()) {
            QVariantMap link(variant.toMap());

            // Cope properly with utopia(s): URLs
            QUrl url(link.value("url").toString());
            if (url.scheme() == "utopiafile") {
                url.setScheme("file");
            } else if (url.scheme().startsWith("utopia")) {
                url.setScheme(url.scheme().replace("utopia", "http"));
                link["url"] = QString::fromUtf8(url.toEncoded());
            }

            links << link;
        }
        qSort(links.begin(), links.end(), link_less_than);
        QUrl objPath(citation->field(Athenaeum::Citation::ObjectFileRole).toUrl());
        if (objPath.isValid()) {
            QVariantMap link;
            link["type"] = "article";
            link["mime"] = "application/pdf";
            link["url"] = objPath;
            links.prepend(link);
        }

        // Make sure there are links to follow
        if (links.isEmpty()) {
            // Without a link there is nothing to do for now
            // FIXME should this method set off resolution?
        } else {
            QVariantMap link(links.takeFirst().toMap());
            QUrl url(QUrl::fromEncoded(link.value("url").toString().toUtf8()));
            bool validate(link.value(":validate", true).toBool());
            d->setState(DownloadingState);
            d->setProgressMsg("Fetching...", url);
            setUrl(url);

            if (validate) {
                QString expects(link.value("mime").toString());
                QNetworkReply * reply = d->networkAccessManager()->get(QNetworkRequest(url));
                reply->setProperty("__originalCitation", QVariant::fromValue< Athenaeum::CitationHandle >(citation));
                reply->setProperty("__originalExpects", expects);
                reply->setProperty("__originalParams", params);
                reply->setProperty("__thisLink", link);
                reply->setProperty("__fallbackLinks", links);
                connect(reply, SIGNAL(finished()), d, SLOT(onNetworkReplyFinished()));
                connect(reply, SIGNAL(downloadProgress(qint64, qint64)), d, SLOT(onNetworkReplyDownloadProgress(qint64, qint64)));
            } else {
                // If no validation is needed, hand it over to the OS
                QDesktopServices::openUrl(url);
                emit closeRequested();
            }
        }
    }

    qreal PapyroTab::progress() const
    {
        return d->progress;
    }

    void PapyroTab::publishChanges()
    {
        d->publishChanges();
    }

    void PapyroTab::quickSearch()
    {
        d->quickSearchBar->focus();
    }

    void PapyroTab::quickSearchNext()
    {
        d->onQuickSearchBarNext();
    }

    void PapyroTab::quickSearchPrevious()
    {
        d->onQuickSearchBarPrevious();
    }

    void PapyroTab::requestUrl(const QUrl & url, const QString & target)
    {
        emit urlRequested(url, target);
    }

    void PapyroTab::removeFromLibrary()
    {
        if (d->citation && d->citation->isKnown()) {
            Athenaeum::Bibliography * master = d->libraryModel->master();
            d->citation->setField(Athenaeum::Citation::DateImportedRole, QVariant());
            master->removeItem(d->citation);
        }
    }

    void PapyroTab::resizeEvent(QResizeEvent * event)
    {
        d->lookupWidget->move(rect().bottomRight() - d->lookupWidget->rect().bottomRight());
        QWidget::resizeEvent(event);
    }

    void PapyroTab::setActiveSelectionProcessorAction(SelectionProcessorAction * processorAction)
    {
        if (d->activeSelectionProcessorAction != processorAction) {
            d->activeSelectionProcessorAction = processorAction;
        }
    }

    void PapyroTab::setCitation(Athenaeum::CitationHandle citation)
    {
        if (citation != d->citation) {
            if (d->citation) {
                d->citation->disconnect(d);
            }
            d->citation = citation;
            if (d->citation) {
                connect(d->citation.get(), SIGNAL(changed(int, QVariant)),
                        d, SLOT(onCitationChanged(int, QVariant)));
            }
            emit citationChanged();
        }
    }

    void PapyroTab::setProgress(qreal progress)
    {
        d->progressSpinner->setProgress(progress);
    }

    void PapyroTab::setSelectionProcessorActions(const QList< SelectionProcessorAction * > & processorActions)
    {
        d->selectionProcessorActions = processorActions;
    }

    void PapyroTab::setStarred(bool starred)
    {
        Athenaeum::Bibliography * master = d->libraryModel->master();
        if (!d->citation || !master->itemForKey(d->citation->field(Athenaeum::Citation::KeyRole).toString())) {
            if (!d->citation) {
                // Add to library
                Athenaeum::CitationHandle citation = Athenaeum::CitationHandle(new Athenaeum::Citation);
                Spine::AnnotationSet annotations(d->document()->annotations("Document Metadata"));
                QVariantMap collated;
                QMap< QString, int > weights;
                foreach (Spine::AnnotationHandle annotation, annotations) {
                    if (annotation->getFirstProperty("concept") == "Citation") {
                        QVariantMap mapping(citationToMap(annotation));
                        QMapIterator< QString, QVariant > iter(mapping);
                        while (iter.hasNext()) {
                            iter.next();
                            if (iter.key() == "title" || iter.key() == "authors") {
                                collated[iter.key()] = iter.value();
                            } else if (iter.key() == "identifiers") {
                                QVariantMap new_ids = iter.value().toMap();
                                QVariantMap old_ids = collated[iter.key()].toMap();
                                QMapIterator< QString, QVariant > id_iter(new_ids);
                                while (id_iter.hasNext()) {
                                    id_iter.next();
                                    if (!old_ids.contains(id_iter.key())) {
                                        old_ids[id_iter.key()] = id_iter.value();
                                    }
                                }
                                collated[iter.key()] = old_ids;
                            }
                        }
                    }
                }
                QMapIterator< QString, QVariant > iter(collated);
                while (iter.hasNext()) {
                    iter.next();
                    if (!collated.value("title").toString().isEmpty()) {
                        citation->setField(Athenaeum::Citation::TitleRole, collated.value("title"));
                    }
                    if (!collated.value("authors").toList().isEmpty()) {
                        citation->setField(Athenaeum::Citation::AuthorsRole, collated.value("authors"));
                    }
                    if (!collated.value("identifiers").toMap().isEmpty()) {
                        citation->setField(Athenaeum::Citation::IdentifiersRole, collated.value("identifiers"));
                    }
                }
                setCitation(citation);
            }
            master->appendItem(d->citation);
        }
        if (false && d->citation && master->itemForKey(d->citation->field(Athenaeum::Citation::KeyRole).toString()) == d->citation) {
            Athenaeum::Citation::Flags itemFlags = d->citation->field(Athenaeum::Citation::FlagsRole).value< Athenaeum::Citation::Flags >();
            if (starred) {
                itemFlags |= Athenaeum::Citation::StarredFlag;
            } else {
                itemFlags &= ~Athenaeum::Citation::StarredFlag;
            }
            d->citation->setField(Athenaeum::Citation::FlagsRole, QVariant::fromValue< Athenaeum::Citation::Flags >(itemFlags));
        }
    }

    void PapyroTab::setTitle(const QString & title)
    {
        if (title != d->title) {
            d->title = title;
            emit titleChanged(title);
        }
    }

    void PapyroTab::setUrl(const QUrl & url)
    {
        if (url != d->url) {
            d->url = url;
            emit urlChanged(url);
        }
    }

    void PapyroTab::star()
    {
        if (d->citation && !d->citation->isStarred()) {
            if (!d->citation->isKnown()) {
                addToLibrary();
            }
            Athenaeum::Citation::Flags itemFlags =
                d->citation->field(Athenaeum::Citation::FlagsRole).value< Athenaeum::Citation::Flags >();
            itemFlags |= Athenaeum::Citation::StarredFlag;
            d->citation->setField(Athenaeum::Citation::FlagsRole, QVariant::fromValue< Athenaeum::Citation::Flags >(itemFlags));
        }
    }

    void PapyroTab::unstar()
    {
        if (d->citation && d->citation->isStarred()) {
            Athenaeum::Citation::Flags itemFlags =
                d->citation->field(Athenaeum::Citation::FlagsRole).value< Athenaeum::Citation::Flags >();
            itemFlags &= ~Athenaeum::Citation::StarredFlag;
            d->citation->setField(Athenaeum::Citation::FlagsRole, QVariant::fromValue< Athenaeum::Citation::Flags >(itemFlags));
        }
    }

    PapyroTab::State PapyroTab::state() const
    {
        return d->state;
    }

    QString PapyroTab::title() const
    {
        return d->title;
    }

    QUrl PapyroTab::url() const
    {
        return d->url;
    }

    void PapyroTab::visualiseAnnotations(const Spine::AnnotationSet & annotations)
    {
        d->visualiseAnnotations(annotations, QVariantMap());
    }

} // namespace Papyro


