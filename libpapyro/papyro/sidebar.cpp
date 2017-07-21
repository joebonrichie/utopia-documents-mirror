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

#include <papyro/sidebar_p.h>
#include <papyro/sidebar.h>

#include <papyro/documentproxy.h>
#include <papyro/papyrowindow.h>
#include <papyro/citations.h>
#include <papyro/resultsview.h>
#include <papyro/utils.h>
#include <utopia2/qt/hidpi.h>
#include <utopia2/qt/elidedlabel.h>
#include <utopia2/qt/slidelayout.h>
#include <utopia2/qt/spinner.h>
#include <utopia2/qt/webview.h>
#include <papyro/citation.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStackedLayout>
#include <QWebFrame>

#include <QDebug>

namespace Papyro
{

    SidebarPrivate::SidebarPrivate(Sidebar * sidebar)
        : QObject(sidebar), sidebar(sidebar), expectingMore(false), documentProxy(0)
    {
        connect(this, SIGNAL(termExplored(const QString &)),
                sidebar, SIGNAL(termExplored(const QString &)));
    }

    void SidebarPrivate::linkClicked(const QUrl & href, const QString & target)
    {
        if (target == "sidebar" && !href.isRelative()) {
            QNetworkRequest request(href);
            request.setRawHeader("User-Agent", webView->userAgentForUrl(href).toUtf8());
            QNetworkReply * reply = networkAccessManager()->get(request);
            reply->setProperty("__target", target);
            connect(reply, SIGNAL(finished()), this, SLOT(linkClickedFinished()));
        } else {
            emit urlRequested(href, target);
        }
    }

    void SidebarPrivate::linkClickedFinished()
    {
        QNetworkReply * reply = static_cast< QNetworkReply * >(sender());

        QString target = reply->property("__target").toString();
        QVariant redirectsVariant = reply->property("__redirects");
        int redirects = redirectsVariant.isNull() ? 20 : redirectsVariant.toInt();

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
                reply->setProperty("__target", target);
                connect(reply, SIGNAL(finished()), this, SLOT(linkClickedFinished()));
            } else {
                // TOO MANY REDIRECTS
            }
            reply->deleteLater();
            return;
        }

        // Check headers... if PDF then launch a new window, otherwise give it to the OS
        QString contentType(reply->header(QNetworkRequest::ContentTypeHeader).toString());
        if (contentType.contains("application/pdf")) {
            emit urlRequested(reply->request().url(), "tab");
        } else {
            QUrl href(reply->request().url());
            if (href.isValid()) {
                if (target == "sidebar") {
                    if (href.scheme() == "http" || href.scheme() == "https" || href.scheme() == "data") {
                        webView->setUrl(href);
                        slideLayout->push("web");
                        return;
                    }
                }

                QDesktopServices::openUrl(href);
            }
            // FIXME error
        }

        reply->deleteLater();
    }

    void SidebarPrivate::onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added)
    {
        bool wasEmpty = citationLists.isEmpty();
        if (added) {
            if (name == "__master_accumulator_list__") {
                foreach (Spine::AnnotationHandle link, annotations) {
                    if (link->getFirstProperty("concept") == "AccumulatorListLink" &&
                        link->getFirstProperty("type") == "citation_list") {
                        QString scratch(qStringFromUnicode(link->getFirstProperty("scratch")));
                        Athenaeum::Bibliography * bibliography = new Athenaeum::Bibliography(this);
                        QString title(qStringFromUnicode(link->getFirstProperty("property:list_name")));
                        bibliography->setTitle(title);
                        citationLists[scratch] = bibliography;
                        // Add to combo box!
                        listLabel->setText(title);
                        listComboBox->addItem(title, scratch);
                    }
                }
            } else if (citationLists.contains(qStringFromUnicode(name))) {
                Athenaeum::Bibliography * bibliography = citationLists[qStringFromUnicode(name)];
                foreach (Spine::AnnotationHandle annotation, annotations) {
                    if (annotation->getFirstProperty("concept") == "Citation") {
                        Athenaeum::CitationHandle citation = Athenaeum::Citation::fromMap(citationToMap(annotation));
                        bibliography->appendItem(citation);
                    }
                }
            }
        }
        if (!citationLists.isEmpty()) {
            if (wasEmpty) {
                onHeaderLabelLinkActivated("summary");
                citationListView->setModel(citationLists.begin().value());
                listComboBox->hide();
                listLabel->show();
            } else if (citationLists.size() > 1) {
                listComboBox->show();
                listLabel->hide();
            }
        } else {
            listLabel->hide();
            listComboBox->hide();
        }
    }

    void SidebarPrivate::onListComboBoxCurrentIndexChanged(int index)
    {
        QString scratch = listComboBox->itemData(index).toString();
        if (Athenaeum::Bibliography * bibliography = citationLists.value(scratch, 0)) {
            citationListView->setModel(bibliography);
        }
    }

    void SidebarPrivate::onLookupBoxActivated()
    {
        sidebar->setMode(Sidebar::Results);
        sidebar->resultsView()->clear();
        sidebar->resultsView()->setExploreTerm(lookupLineEdit->text(), true);
    }

    void SidebarPrivate::onHeaderLabelLinkActivated(const QString & link)
    {
        headerLabel->setText("<span style='color:#bbb'><span style='color:black'>Summary</span>");
        documentWideStackedLayout->setCurrentIndex(0);
        return;

        // FIXME it should really be like the below, but we'll turn this on once the citation lists are fully functional

        if (citationLists.isEmpty()) {
            headerLabel->setText("<span style='color:#bbb'><span style='color:black'>Summary</span> | Citations</span>");
            documentWideStackedLayout->setCurrentIndex(0);
        } else if (link == "summary") {
            headerLabel->setText("<span style='color:#bbb'><span style='color:black'>Summary</span> | <a href='citations'>Citations</a></span>");
            documentWideStackedLayout->setCurrentIndex(0);
        } else if (link == "citations") {
            headerLabel->setText("<span style='color:#bbb'><a href='summary'>Summary</a> | <span style='color:black'>Citations</span></span>");
            documentWideStackedLayout->setCurrentIndex(1);
        }
    }

    void SidebarPrivate::onResultsViewRunningChanged(bool running)
    {
        updateSpinner();
    }

    void SidebarPrivate::updateSpinner()
    {
        if (resultsViewSpinner->active()) {
            if (!expectingMore && !resultsView->isRunning()) {
                resultsViewSpinner->stop();
            }
        } else {
            if (expectingMore || resultsView->isRunning()) {
                resultsViewSpinner->start();
            }
        }
    }




    Sidebar::Sidebar(QWidget * parent)
        : QFrame(parent), d(new SidebarPrivate(this))
    {
        int minWidth = sizeHint().width();
        int maxWidth = minWidth;
#ifndef Q_OS_MAC
        {
            QWidget * screen = QApplication::desktop()->screen();
            maxWidth = qMax(screen->width() / 3, minWidth);
        }
#endif
        setMinimumWidth(minWidth);
        setMaximumWidth(maxWidth);

        connect(d, SIGNAL(urlRequested(const QUrl &, const QString &)), this, SIGNAL(urlRequested(const QUrl &, const QString &)));

        // Construct sidebar
        d->slideLayout = new Utopia::SlideLayout(Utopia::SlideLayout::StackRight, this);

        {
            d->documentWideFrame = new QFrame;
            QVBoxLayout * documentWideLayout = new QVBoxLayout(d->documentWideFrame);
            documentWideLayout->setContentsMargins(0, 0, 0, 0);
            documentWideLayout->setSpacing(0);
            QFrame * headerFrame = new QFrame;
            QHBoxLayout * headerFrameLayout = new QHBoxLayout(headerFrame);
            headerFrameLayout->setSpacing(6);
            headerFrameLayout->setContentsMargins(6, 0, 6, 0);
            headerFrame->setObjectName("document_wide_header");
            d->headerLabel = new QLabel;
            connect(d->headerLabel, SIGNAL(linkActivated(const QString &)),
                    d, SLOT(onHeaderLabelLinkActivated(const QString &)));
            headerFrameLayout->addWidget(d->headerLabel, 0, Qt::AlignCenter);

            d->documentWideView = new ResultsView("document-wide");
            d->documentWideView->setObjectName("document-wide");
            connect(d->documentWideView, SIGNAL(citationsActivated(const QVariantList &, const QString &)),
                    this, SIGNAL(citationsActivated(const QVariantList &, const QString &)));
            connect(d->documentWideView, SIGNAL(linkClicked(const QUrl &, const QString &)),
                    d, SLOT(linkClicked(const QUrl &, const QString &)));
            connect(d->documentWideView, SIGNAL(selectionChanged()),
                    this, SLOT(onSelectionChanged()));

            QFrame * frame = new QFrame;
            QVBoxLayout * layout = new QVBoxLayout(frame);
            layout->setSpacing(0);
            layout->setContentsMargins(0, 0, 0, 0);
            d->listLabel = new QLabel;
            d->listLabel->setObjectName("citation_list_name");
            layout->addWidget(d->listLabel, 0, Qt::AlignCenter);
            d->listComboBox = new QComboBox;
            d->listComboBox->setObjectName("citation_list_box");
            connect(d->listComboBox, SIGNAL(currentIndexChanged(int)),
                    d, SLOT(onListComboBoxCurrentIndexChanged(int)));
            layout->addWidget(d->listComboBox, 0, Qt::AlignCenter);

            d->documentWideStackedLayout = new QStackedLayout;
            d->documentWideStackedLayout->addWidget(d->documentWideView);
            d->documentWideStackedLayout->addWidget(frame);
            d->documentWideStackedLayout->setContentsMargins(0, 0, 0, 0);

            d->citationListView = new Athenaeum::ArticleView;
            layout->addWidget(d->citationListView, 1);

            documentWideLayout->addWidget(headerFrame, 0);
            documentWideLayout->addLayout(d->documentWideStackedLayout, 1);

            QFrame * exploreFrame = new QFrame;
            exploreFrame->setObjectName("lookup-search-box");
            QHBoxLayout * exploreLayout = new QHBoxLayout(exploreFrame);
            exploreLayout->setContentsMargins(8, 8, 8, 8);
            exploreLayout->setSpacing(0);
            exploreLayout->addWidget(d->lookupLineEdit = new QLineEdit);
            d->lookupLineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);
            d->lookupLineEdit->setPlaceholderText("Explore...");
            connect(d->lookupLineEdit, SIGNAL(returnPressed()), d, SLOT(onLookupBoxActivated()));
            documentWideLayout->addWidget(exploreFrame);

            d->onHeaderLabelLinkActivated("summary");

            d->slideLayout->addWidget(d->documentWideFrame, "documentwide");
        }

        d->resultsView = new ResultsView("results-view");
        d->resultsView->setObjectName("results-view");
        connect(d->resultsView, SIGNAL(citationsActivated(const QVariantList &, const QString &)),
                this, SIGNAL(citationsActivated(const QVariantList &, const QString &)));
        connect(d->resultsView, SIGNAL(linkClicked(const QUrl &, const QString &)),
                d, SLOT(linkClicked(const QUrl &, const QString &)));
        connect(d->resultsView, SIGNAL(runningChanged(bool)),
                d, SLOT(onResultsViewRunningChanged(bool)));
        connect(d->resultsView, SIGNAL(selectionChanged()),
                this, SLOT(onSelectionChanged()));
        connect(d->resultsView, SIGNAL(termExplored(const QString &)),
                this, SIGNAL(termExplored(const QString &)));
        d->resultsViewWidget = new QWidget;
        QVBoxLayout * resultsViewLayout = new QVBoxLayout(d->resultsViewWidget);
        resultsViewLayout->setContentsMargins(0, 0, 0, 0);
        resultsViewLayout->setSpacing(0);
        QFrame * headerFrame = new QFrame;
        headerFrame->setObjectName("results-view-header");
        QHBoxLayout * headerLayout = new QHBoxLayout(headerFrame);
        headerLayout->setContentsMargins(6, 0, 6, 0);
        headerLayout->setSpacing(6);
        QPushButton * backButton = new QPushButton("Back");
        headerLayout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
        d->searchTermLabel = new Utopia::ElidedLabel;
        d->searchTermLabel->hide();
        d->searchTermLabel->setAlignment(Qt::AlignCenter);
        headerLayout->addWidget(d->searchTermLabel, 1, Qt::AlignCenter);
        d->resultsViewSpinner = new Utopia::Spinner;
        d->resultsViewSpinner->setFixedSize(4, 4);
        headerLayout->addWidget(d->resultsViewSpinner, 0, Qt::AlignRight | Qt::AlignVCenter);
        connect(backButton, SIGNAL(clicked()), d->slideLayout, SLOT(pop()));
        resultsViewLayout->addWidget(headerFrame, 0);
        resultsViewLayout->addWidget(d->resultsView, 1);
        d->slideLayout->addWidget(d->resultsViewWidget, "results");

        d->webView = new Utopia::WebView;
        connect(d->webView, SIGNAL(linkClicked(const QUrl &)), d, SLOT(linkClicked(const QUrl &)));
        connect(d->webView, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
        QWidget * webViewWidget = new QWidget;
        QVBoxLayout * webViewLayout = new QVBoxLayout(webViewWidget);
        webViewLayout->setContentsMargins(0, 0, 0, 0);
        webViewLayout->setSpacing(0);
        headerFrame = new QFrame;
        headerFrame->setObjectName("web-page-header");
        headerLayout = new QHBoxLayout(headerFrame);
        headerLayout->setContentsMargins(6, 0, 6, 0);
        headerLayout->setSpacing(6);
        backButton = new QPushButton("Back");
        headerLayout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
        Utopia::Spinner * hidden = new Utopia::Spinner;
        hidden->setFixedSize(4, 4);
        headerLayout->addWidget(hidden, 0, Qt::AlignRight | Qt::AlignVCenter);
        connect(backButton, SIGNAL(clicked()), d->slideLayout, SLOT(pop()));
        webViewLayout->addWidget(headerFrame, 0);
        webViewLayout->addWidget(d->webView, 1);
        d->slideLayout->addWidget(webViewWidget, "web");

        d->slideLayout->push("documentwide", false);
    }

    void Sidebar::clear()
    {
        d->resultsView->clear();
        d->searchTermLabel->setText(QString());
        d->documentWideView->clear();
        d->webView->setContent(QByteArray());
        while (d->slideLayout->top() && d->slideLayout->top() != d->documentWideFrame) {
            d->slideLayout->pop();
        }
    }

    void Sidebar::copySelectedText()
    {
        QWebView * webView = 0;
        if ((webView = qobject_cast< QWebView * >(d->slideLayout->top())) ||
            (webView = d->slideLayout->top()->findChild< QWebView * >())) {
            webView->triggerPageAction(QWebPage::Copy);
        }
    }

    ResultsView * Sidebar::documentWideView() const
    {
        return d->documentWideView;
    }

    void Sidebar::lookupStarted()
    {
        d->expectingMore = true;
        d->updateSpinner();
    }

    void Sidebar::lookupStopped()
    {
        d->expectingMore = false;
        d->updateSpinner();
    }

    void Sidebar::onSelectionChanged()
    {
        QWebView * sender = qobject_cast< QWebView * >(this->sender());
        if (sender != d->documentWideView) { d->documentWideView->findText(""); }
        if (sender != d->resultsView) { d->resultsView->findText(""); }
        if (sender != d->webView) { d->webView->findText(""); }
        if (sender && !sender->selectedText().isEmpty()) { emit selectionChanged(); }
    }

    ResultsView * Sidebar::resultsView() const
    {
        return d->resultsView;
    }

    void Sidebar::setDocumentProxy(DocumentProxy * documentProxy)
    {
        if (d->documentProxy) {
            disconnect(d->documentProxy, SIGNAL(annotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)),
                       d, SLOT(onDocumentAnnotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)));
        }

        d->documentProxy = documentProxy;

        if (d->documentProxy) {
            connect(d->documentProxy, SIGNAL(annotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)),
                    d, SLOT(onDocumentAnnotationsChanged(const std::string &, const Spine::AnnotationSet &, bool)));
        }
    }

    void Sidebar::setMode(SidebarMode mode)
    {
        QWidget * top = 0;;

        switch (mode) {
        case DocumentWide:
            while ((top = d->slideLayout->top())) {
                if (top == d->documentWideFrame) {
                    break;
                }
                d->slideLayout->pop();
            }
            if (top != d->documentWideFrame) {
                d->slideLayout->push("documentwide");
            }
            break;
        case Results:
            while ((top = d->slideLayout->top())) {
                if (top == d->documentWideFrame || top == d->resultsViewWidget) {
                    break;
                }
                d->slideLayout->pop();
            }
            if (top != d->resultsViewWidget) {
                d->slideLayout->push("results");
            }
            break;
        default:
            break;
        }
    }

    void Sidebar::setSearchTerm(const QString & term)
    {
        d->searchTermLabel->setText(term);
    }

    QSize Sidebar::sizeHint() const
    {
        return QSize(qMin(Utopia::maxScreenWidth() / 3, qRound(320 * Utopia::hiDPIScaling())), 0);
    }

    Utopia::WebView * Sidebar::webView() const
    {
        return d->webView;
    }

}
