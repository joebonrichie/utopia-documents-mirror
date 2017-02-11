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

#include "webview.h"
#include "hidpi.h"
#include "version_p.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QWebPage>
#include <QWebInspector>
#include <QWebFrame>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QDebug>

namespace Utopia
{

    WebPage::WebPage(QObject * parent)
        : QWebPage(parent)
    {
#ifdef Q_OS_WIN
        char env[1024] = { 0 };
        int status = GetEnvironmentVariable("UTOPIA_WEBKIT_INSPECTOR", env, sizeof(env));
        if (status == 0) { env[0] = 0; }
        if (env[0] && strcmp(env, "0") != 0) {
#else
        char * env = ::getenv("UTOPIA_WEBKIT_INSPECTOR");
        if (env && strcmp(env, "0") != 0) {
#endif
            settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
            QWebInspector * inspector = new QWebInspector(0);
            inspector->setPage(this);
        }

        setNetworkAccessManager(NetworkAccessManagerMixin::networkAccessManager().get());
        setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    }

    WebPage::~WebPage()
    {}

    QString WebPage::userAgentForUrl(const QUrl & url) const
    {
        QString userAgent(QWebPage::userAgentForUrl(url) + QString(" Version/" UTOPIA_VERSION_PATCH_STRING));
        //userAgent.replace(" Safari", " Mobile Safari");
        userAgent.replace(" Safari", " Mobile");
        //qDebug() << "userAgent:" << userAgent;
        return userAgent;
    }




    WebView::WebView(QWidget * parent)
        : QWebView(parent), _page(new WebPage(this))
    {
        connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

        connect(_page, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
        setPage(_page);

#ifdef Q_OS_MAC
        setStyle(QStyleFactory::create("Fusion"));
#endif
    }

    WebView::~WebView()
    {}

    void WebView::contextMenuEvent(QContextMenuEvent * e)
    {
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled)) {
            // Don't mask normal functionality if extras are enabled
            QWebView::contextMenuEvent(e);
        }
    }

    void WebView::focusOutEvent(QFocusEvent * e)
    {
        findText("");
        QWebView::focusOutEvent(e);
    }

    void WebView::hideEvent(QHideEvent * e)
    {
        findText("");
        QWebView::hideEvent(e);
    }

    void WebView::onLoadFinished(bool)
    {
        QString family(window()->font().family());
        int pointSize = window()->font().pointSize();

#ifdef Q_OS_MAC
        // There's an odd font name bug on OS X
        if (family == ".Lucida Grande UI") {
            family = "Lucida Grande";
        }
#endif

        // hidpi screens (or artificially increased DPI) should
        // cause the webview to zoom
        qreal dpiScaling = hiDPIScaling();
        setZoomFactor(dpiScaling);
        page()->mainFrame()->evaluateJavaScript(QString("window.devicePixelRatio = %1;").arg(dpiScaling));

#ifdef Q_OS_MAC
        // On Mac, QtWebKit expects the font size in points. Lower it slightly
        // to compensate for a relatively large standard font size
        pointSize = qRound(pointSize * 12.0 / 13.0);
#else
        // On other systems it expects pixels for some reason, so convert first.
        pointSize = qRound(pointSize * 96.0 / 72.0);
#endif

        settings()->setFontSize(QWebSettings::DefaultFontSize, pointSize);
        settings()->setFontFamily(QWebSettings::StandardFont, family);
        settings()->setFontFamily(QWebSettings::SansSerifFont, family);
        settings()->setFontFamily(QWebSettings::SerifFont, family);
        settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
        settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    }

    void WebView::setPage(WebPage * page)
    {
        QWebView::setPage(page);
        onLoadFinished(true);
    }

    QString WebView::userAgentForUrl(const QUrl & url)
    {
        return _page->userAgentForUrl(url);
    }

}
