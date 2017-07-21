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

#include <papyro/capabilities.h>
#include <papyro/cslengineadapter.h>
#include <papyro/resultitem.h>
#include <papyro/papyrotab.h>
#include <papyro/resultsview_p.h>
#include <papyro/resultsview.h>
#include <papyro/resolverrunnable.h>
#include <utopia2/qt/bubble.h>
#include "version_p.h"

#include <QFile>
#include <QWebFrame>

#include <QDebug>

namespace Papyro
{

    template< typename T >
    static T * ancestor(QObject * descendent)
    {
        QObject * parent = descendent;
        while (parent && (parent = parent->parent())) {
            if (T * candidate = qobject_cast< T * >(parent)) {
                return candidate;
            }
        }
        return (T *) 0;
    }




    ResultItemControl::ResultItemControl(ResultsViewPrivate * viewPrivate, ResultItem * item)
        : QObject(viewPrivate), Utopia::BusAgent()
    {
        d.viewPrivate = viewPrivate;
        d.item = item;

        item->setParent(this);

        // Check for capabilities
        d.capabilities.downloads = item->capabilities< DownloadCapability >();

        qRegisterMetaType< Papyro::ResultItem::State >("Papyro::ResultItem::State");
        connect(item, SIGNAL(contentChanged(const QString &)), this, SLOT(contentChanged(const QString &)));
        connect(item, SIGNAL(stateChanged(Papyro::ResultItem::State)), this, SLOT(stateChanged(Papyro::ResultItem::State)));

        // Set bus accordingly
        if (PapyroTab * tab = ancestor< PapyroTab >(this)) {
            setBus(tab->bus());
        }
    }

    QString ResultItemControl::authorUri() const
    {
        return item()->authorUri();
    }

    void ResultItemControl::contentChanged(const QString & key)
    {
        emit insertContent(d.resultElement, item()->content(key));
    }

    QVariantMap ResultItemControl::context() const
    {
        return item()->context();
    }

    QString ResultItemControl::cssId() const
    {
        return item()->cssId();
    }

    QString ResultItemControl::description() const
    {
        return item()->description();
    }

    QWebElement ResultItemControl::element() const
    {
        return d.resultElement;
    }

    bool ResultItemControl::headless() const
    {
        return item()->headless();
    }

    QString ResultItemControl::highlight() const
    {
        return item()->highlight();
    }

    ResultItem * ResultItemControl::item() const
    {
        return d.item;
    }

    bool ResultItemControl::openByDefault() const
    {
        return item()->defaultness();
    }

    void ResultItemControl::postMessage(const QVariant & data)
    {
        postToBus(data);
        //qDebug() << "postMessage" << data;
    }

    void ResultItemControl::postMessage(const QString & recipient, const QVariant & data)
    {
        postToBus(recipient, data);
        //qDebug() << "postMessage" << recipient << data;
    }

    void ResultItemControl::setElement(QWebElement element)
    {
        d.resultElement = element;
        d.resultElement.setAttribute("id", cssId());
    }

    QString ResultItemControl::sourceDatabase() const
    {
        return item()->sourceDatabase();
    }

    QString ResultItemControl::sourceIcon() const
    {
        return QString::fromUtf8(item()->sourceIcon().toEncoded());
    }

    void ResultItemControl::stateChanged(ResultItem::State state)
    {
        // Turn off spinner once done
        if (state == ResultItem::Generated) {
            element().evaluateJavaScript("utopia.onResultItemContentFinished(this)");
        }
    }

    QWebElement ResultItemControl::thumbnailElement(const QString & thumbnailClass) const
    {
        return element().findFirst(".-papyro-internal-graphics img.-papyro-internal-" + thumbnailClass);
    }

    QString ResultItemControl::title() const
    {
        return item()->title();
    }

    void ResultItemControl::toggleContent()
    {
        // If no summary yet generated, generate it
        if (item()->contentState() == ResultItem::Ungenerated) {
            d.contentKeys = item()->contentKeys();
            item()->generateContent();
        }

        // Toggle
        element().evaluateJavaScript("utopia.toggleSlide(this)");
    }

    QString ResultItemControl::value(const QString & key) const
    {
        return item()->value(key);
    }

    QStringList ResultItemControl::values(const QString & key) const
    {
        return item()->values(key);
    }

    int ResultItemControl::weight() const
    {
        return item()->weight();
    }




    MetadataResolutionFuture::MetadataResolutionFuture(const QVariantMap & metadata, const QString & purpose)
        : _ready(false)
    {
        Athenaeum::Resolver::Purposes purposes;
        if (purpose == "identify") { purposes |= Athenaeum::Resolver::Identify; }
        else if (purpose == "expand") { purposes |= Athenaeum::Resolver::Expand; }
        else if (purpose == "dereference") { purposes |= Athenaeum::Resolver::Dereference; }
        else { qDebug() << QString("Error: citation resolution purpose (%1) not recognised.").arg(purpose); }
        Athenaeum::ResolverRunnable::resolve(Athenaeum::Citation::fromMap(metadata), this, SLOT(onResolverRunnableCompleted(Athenaeum::CitationHandle)), purposes);
    }

    void MetadataResolutionFuture::doom()
    {
        deleteLater();
    }

    bool MetadataResolutionFuture::isReady() const
    {
        return _ready;
    }

    void MetadataResolutionFuture::lock()
    {
        _mutex.lock();
    }

    void MetadataResolutionFuture::onResolverRunnableCompleted(Athenaeum::CitationHandle citation)
    {
        lock();
        QVariantMap metadata = citation->toMap();
        emit completed(metadata);
        _metadata = metadata;
        _ready = true;
        unlock();
    }

    QVariantMap MetadataResolutionFuture::results() const
    {
        return _metadata;
    }

    void MetadataResolutionFuture::unlock()
    {
        _mutex.unlock();
    }




    ResultsViewControl::ResultsViewControl(ResultsViewPrivate * resultsViewPrivate)
        : QObject(resultsViewPrivate), d(resultsViewPrivate)
    {}

    void ResultsViewControl::activateCitation(const QVariantMap & citation, const QString & target)
    {
        QVariantList citations;
        citations << citation;
        activateCitations(citations, target);
    }

    void ResultsViewControl::activateCitations(const QVariantList & citations, const QString & target)
    {
        emit citationsActivated(citations, target);
    }

    void ResultsViewControl::activateLink(const QString & href, const QString & target)
    {
        emit linkClicked(QUrl::fromEncoded(href.toUtf8()), target);
    }

    void ResultsViewControl::activateSource(QObject * obj)
    {
        if (ResultItemControl * itemControl = qobject_cast< ResultItemControl * >(obj)) {
            QWebElement element = itemControl->thumbnailElement("source");
            if (ResultItem * item = itemControl->item()) {
                Utopia::BubbleWidget * bubble = new Utopia::BubbleWidget(0, Qt::Popup);
                bubble->setObjectName("source");
                bubble->setAttribute(Qt::WA_DeleteOnClose, true);
                bubble->setPopup(true);
                QHBoxLayout * l = new QHBoxLayout(bubble);
                QLabel * label = new QLabel(QString(
                    "<html>"
                      "<head>"
                        "<style type=text/css>"
                          "a:link { text-decoration: none; color: #067; } "
                          "a:hover { text-decoration: underline; color: #067; } "
                          ".right { text-align: right; }"
                        "</style>"
                      "</head>"
                      "<body>"
                        "%1"
                      "</body>"
                    "</html>"
                    ).arg(item->sourceDescription()));
                label->setWordWrap(true);
                label->setTextFormat(Qt::RichText);
                label->setOpenExternalLinks(true);
                l->addWidget(label);
                l->setContentsMargins(0, 0, 0, 0);
                QRect geometry(element.geometry());
                QPoint bottomCenter = QPoint(geometry.left() + (geometry.width() / 4), geometry.bottom());
                QPoint viewTopRight = d->view->mapToGlobal(QPoint(d->view->width(), 0));
                bubble->setCorners(Utopia::AllCorners);
                bubble->setCalloutSide(Utopia::TopCallout);
                bubble->setCalloutPosition(bottomCenter.x() - 106);
                bubble->setFixedWidth(d->view->width() - 120);
                bubble->setFixedHeight(bubble->heightForWidth(bubble->width()));
                bubble->move(viewTopRight.x() - 10 - bubble->width(), viewTopRight.y() + bottomCenter.y() - element.evaluateJavaScript("$(window).scrollTop()").toInt());
                bubble->show();
            }
        }
    }

    QVariantMap ResultsViewControl::availableCitationStyles()
    {
        return d->cslengine->availableStyles();
    }

    QString ResultsViewControl::defaultCitationStyle()
    {
        return d->cslengine->defaultStyle();
    }

    void ResultsViewControl::explore(const QString & term)
    {
        // Set off an explore
        emit termExplored(term);
    }

    QString ResultsViewControl::formatCitation(const QVariantMap & metadata, const QString & style)
    {
        return d->cslengine->format(convert_to_cslengine(metadata), style);
    }

    void ResultsViewControl::onLoadComplete()
    {
        //qDebug() << "ResultsViewControl::onLoadComplete()";
        d->ready = true;
        d->wait.quit();

        QStringList encoded;
        if (!d->terms.isEmpty()) {
            foreach (QString term, d->terms) {
                encoded << term.replace("\\", "\\\\").replace("'", "\'");
            }
            QString command = "jQuery(function () { utopia.setExploreTerms(['" + encoded.join("', '") + "'], " + QString(d->exploreTerms ? "true" : "false") + "); });";
            d->view->page()->mainFrame()->evaluateJavaScript(command);
        }
    }

    QObject * ResultsViewControl::resolveMetadata(const QVariantMap & metadata, const QString & purpose)
    {
        MetadataResolutionFuture * future = new MetadataResolutionFuture(metadata, purpose);
        return future;
    }

    void ResultsViewControl::searchRemote(const QString & term)
    {
        // Set bus accordingly
        if (!bus()) {
            if (PapyroTab * tab = ancestor< PapyroTab >(this)) {
                setBus(tab->bus());
            }
        }

        if (bus()) {
            QVariantMap data;
            data["term"] = term;
            data["action"] = "searchRemote";
            postToBus("window", data);
        }
    }




    ResultsViewPrivate::ResultsViewPrivate(ResultsView * view)
        : QObject(view), view(view), control(new ResultsViewControl(this)), cslengine(CSLEngine::instance()), ready(false)
    {
        // Result queue
        resultQueueTimer.setInterval(30);
        resultQueueTimer.setSingleShot(true);
        connect(&resultQueueTimer, SIGNAL(timeout()), this, SLOT(addResult()));

        connect(this, SIGNAL(resultAdded(QObject*)),
                control, SIGNAL(resultAdded(QObject*)));
        connect(control, SIGNAL(linkClicked(const QUrl &, const QString &)),
                this, SIGNAL(linkClicked(const QUrl &, const QString &)));
        connect(control, SIGNAL(citationsActivated(const QVariantList &, const QString &)),
                view, SIGNAL(citationsActivated(const QVariantList &, const QString &)));
        connect(control, SIGNAL(termExplored(const QString &)),
                view, SIGNAL(termExplored(const QString &)));
    }

    void ResultsViewPrivate::addResult()
    {
        if (!resultQueue.isEmpty()) {
            ResultItemControl * result = new ResultItemControl(this, resultQueue.dequeue());

            // Store new result
            results.append(result);

            if (!resultQueue.isEmpty()) {
                resultQueueTimer.start();
            } else {
                emit runningChanged(false);
            }

            // Wait in case the page is still loading
            if (!ready) {
                //qDebug() << "ResultsViewPrivate::addResult() --- waiting";
                wait.exec();
            }
            //qDebug() << "ResultsViewPrivate::addResult()";

            // Send result into the web view
            emit resultAdded(result);
        }
    }

    void ResultsViewPrivate::setupJavaScriptWindowObject()
    {
        // Attach sidebar control
        view->page()->mainFrame()->addToJavaScriptWindowObject("control", control);
        view->page()->mainFrame()->evaluateJavaScript(
            "window.onload = function() { jQuery('body').addClass('" + classes.join(" ").replace("\\", "\\\\").replace("'", "\'") + "'); }"
        );
    }




    ResultsView::ResultsView(const QString & classes, QWidget * parent)
        : Utopia::WebView(parent), d(new ResultsViewPrivate(this))
    {
        qRegisterMetaType<QWebElement>("QWebElement");
        setRenderHint(QPainter::Antialiasing, true);
        setRenderHint(QPainter::TextAntialiasing, true);
        setRenderHint(QPainter::SmoothPixmapTransform, true);

        connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), d, SLOT(setupJavaScriptWindowObject()));
        connect(d, SIGNAL(linkClicked(const QUrl &, const QString &)), this, SIGNAL(linkClicked(const QUrl &, const QString &)));
        connect(d, SIGNAL(runningChanged(bool)), this, SIGNAL(runningChanged(bool)));

        d->classes = classes.split(QRegExp("\\s"), QString::SkipEmptyParts);

        setUrl(QUrl("qrc:/pages/results.html"));

        page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    }

    void ResultsView::addResult(ResultItem * result)
    {
        result->setParent(this);
        bool wasEmpty = d->resultQueue.isEmpty();
        d->resultQueue.enqueue(result);
        if (!d->resultQueueTimer.isActive()) {
            d->resultQueueTimer.start();
        }
        if (wasEmpty) {
            emit runningChanged(true);
        }
    }

    void ResultsView::clear()
    {
        //QWebSettings::clearMemoryCaches();
        QStringList classes = d->classes;
        page()->mainFrame()->evaluateJavaScript("utopia.clear();");
        d->deleteLater();
        d = new ResultsViewPrivate(this);

        connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), d, SLOT(setupJavaScriptWindowObject()));
        connect(d, SIGNAL(linkClicked(const QUrl &, const QString &)), this, SIGNAL(linkClicked(const QUrl &, const QString &)));
        connect(d, SIGNAL(runningChanged(bool)), this, SIGNAL(runningChanged(bool)));

        d->classes = classes;

        setUrl(QUrl("qrc:/pages/results.html"));
    }

    bool ResultsView::isRunning() const
    {
        return !d->resultQueue.isEmpty();
    }

    void ResultsView::setExploreTerms(const QStringList & terms, bool explore)
    {
        d->terms = terms;
        d->exploreTerms = explore;

        if (d->ready) {
            QStringList encoded;
            foreach (QString term, d->terms) {
                encoded << term.replace("\\", "\\\\").replace("'", "\'");
            }
            QString command = "jQuery(function () { utopia.setExploreTerms(['" + encoded.join("', '") + "'], " + QString(d->exploreTerms ? "true" : "false") + "); });";
            page()->mainFrame()->evaluateJavaScript(command);
        }
    }

    void ResultsView::setExploreTerm(const QString & term, bool explore)
    {
        QStringList terms;
        terms << term;
        setExploreTerms(terms, explore);
    }

}
