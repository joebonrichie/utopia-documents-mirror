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

#include <papyro/sidebar_p.h>
#include <papyro/sidebar.h>

#include <papyro/papyrowindow.h>
#include <papyro/resultsview.h>
#include <utopia2/qt/elidedlabel.h>
#include <utopia2/qt/slidelayout.h>
#include <utopia2/qt/spinner.h>
#include <utopia2/qt/webview.h>

#include <QAction>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QWebFrame>

#include <QDebug>

namespace Papyro
{

    SidebarPrivate::SidebarPrivate(Sidebar * sidebar)
        : QObject(sidebar), sidebar(sidebar), expectingMore(false)
    {}

    void SidebarPrivate::linkClicked(const QUrl & href, const QString & target)
    {
        if (target == "sidebar") {
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
        if (redirectedUrl.isValid())
        {
            if (redirectedUrl.isRelative())
            {
                QUrl oldUrl = reply->url();
                redirectedUrl.setScheme(oldUrl.scheme());
                redirectedUrl.setAuthority(oldUrl.authority());
            }
            if (redirects > 0)
            {
                QNetworkRequest request = reply->request();
                request.setUrl(redirectedUrl);
                QNetworkReply * reply = networkAccessManager()->get(request);
                reply->setProperty("__target", target);
                connect(reply, SIGNAL(finished()), this, SLOT(linkClickedFinished()));
            }
            else
            {
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
                if (href.scheme() == "http" || href.scheme() == "https") {
                    if (target == "sidebar") {
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
        connect(d, SIGNAL(urlRequested(const QUrl &, const QString &)), this, SIGNAL(urlRequested(const QUrl &, const QString &)));

        // Construct sidebar
        d->slideLayout = new Utopia::SlideLayout(Utopia::SlideLayout::StackRight, this);

        d->documentWideView = new ResultsView;
        connect(d->documentWideView, SIGNAL(linkClicked(const QUrl &, const QString &)), d, SLOT(linkClicked(const QUrl &, const QString &)));
        connect(d->documentWideView, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
        d->slideLayout->addWidget(d->documentWideView, "documentwide");

        d->resultsView = new ResultsView;
        connect(d->resultsView, SIGNAL(linkClicked(const QUrl &, const QString &)), d, SLOT(linkClicked(const QUrl &, const QString &)));
        connect(d->resultsView, SIGNAL(runningChanged(bool)), d, SLOT(onResultsViewRunningChanged(bool)));
        connect(d->resultsView, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
        d->resultsViewWidget = new QWidget;
        QVBoxLayout * resultsViewLayout = new QVBoxLayout(d->resultsViewWidget);
        resultsViewLayout->setContentsMargins(0, 0, 0, 0);
        resultsViewLayout->setSpacing(0);
        QFrame * headerFrame = new QFrame;
        QHBoxLayout * headerLayout = new QHBoxLayout(headerFrame);
        headerLayout->setContentsMargins(0, 0, 0, 0);
        QPushButton * backButton = new QPushButton("Back");
        backButton->setFlat(true);
        backButton->setObjectName("back");
        headerLayout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
        d->searchTermLabel = new Utopia::ElidedLabel;
        d->searchTermLabel->setAlignment(Qt::AlignCenter);
        headerLayout->addWidget(d->searchTermLabel, 1);
        d->resultsViewSpinner = new Utopia::Spinner;
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
        headerLayout = new QHBoxLayout(headerFrame);
        headerLayout->setContentsMargins(0, 0, 0, 0);
        backButton = new QPushButton("Back");
        backButton->setFlat(true);
        backButton->setObjectName("back");
        headerLayout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
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
        while (d->slideLayout->top() && d->slideLayout->top() != d->documentWideView) {
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

    void Sidebar::setMode(SidebarMode mode)
    {
        QWidget * top = 0;;

        switch (mode) {
        case DocumentWide:
            while ((top = d->slideLayout->top())) {
                if (top == d->documentWideView) {
                    break;
                }
                d->slideLayout->pop();
            }
            if (top != d->documentWideView) {
                d->slideLayout->push("documentwide");
            }
            break;
        case Results:
            while ((top = d->slideLayout->top())) {
                if (top == d->documentWideView || top == d->resultsViewWidget) {
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

    Utopia::WebView * Sidebar::webView() const
    {
        return d->webView;
    }

}
