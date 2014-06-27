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

#include <utopia2/qt/webview.h>
#include <utopia2/networkaccessmanager.h>
#include "version_p.h"

#include <QWebPage>
#include <QWebInspector>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif
//#include <QDebug>

namespace Utopia
{

    class WebPage : public QWebPage, NetworkAccessManagerMixin
    {
    public:
        WebPage(QObject * parent = 0)
            : QWebPage(parent)
        {
            setNetworkAccessManager(NetworkAccessManagerMixin::networkAccessManager().get());
            setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
        }

        QString userAgentForUrl(const QUrl & url) const
        {
            QString userAgent(QWebPage::userAgentForUrl(url) + QString(" Utopia/" UTOPIA_VERSION_PATCH_STRING));
            userAgent.replace(" Safari", " Mobile Safari");
            //qDebug() << "userAgent:" << userAgent;
            return userAgent;
        }
    };




    WebView::WebView(QWidget * parent)
        : QWebView(parent), _page(new WebPage(this)), useInspector(false)
    {
        setPage(_page);
        connect(_page, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
#if defined(Q_OS_MACX)
        //settings()->setFontFamily(QWebSettings::SansSerifFont, "Arial");
#endif

#ifdef Q_OS_WIN32
        char env[1024] = { 0 };
        int status = GetEnvironmentVariable("UTOPIA_WEBKIT_INSPECTOR", env, sizeof(env));
        if (status != 0) { env[0] = 0; }
#else
        char * env = ::getenv("UTOPIA_WEBKIT_INSPECTOR");
#endif
        useInspector = (env && strcmp(env, "0") != 0);

        if (useInspector) {
            page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
            QWebInspector * inspector = new QWebInspector(0);
            inspector->setPage(page());
        }
    }

    WebView::~WebView()
    {}

    void WebView::contextMenuEvent(QContextMenuEvent * e)
    {
        if (useInspector) {
            // Don't mask normal functionality
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

    QString WebView::userAgentForUrl(const QUrl & url)
    {
        return _page->userAgentForUrl(url);
    }

}
