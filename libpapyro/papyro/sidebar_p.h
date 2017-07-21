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

#ifndef PAPYRO_SIDEBAR_P_H
#define PAPYRO_SIDEBAR_P_H

#include <utopia2/networkaccessmanager.h>
#include <papyro/bibliography.h>
#include <papyro/articleview.h>
#include <papyro/documentproxy.h>

#include <QObject>

class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QStackedLayout;
class QPushButton;
class QUrl;

namespace Utopia
{
    class ElidedLabel;
    class SlideLayout;
    class Spinner;
    class WebView;
}

namespace Papyro
{

    class ResultsView;

    class Sidebar;
    class SidebarPrivate : public QObject, public Utopia::NetworkAccessManagerMixin
    {
        Q_OBJECT

    public:
        enum {
            EmptyPane = 0,
            DocumentWidePane,
            ResultsPane,
            WebPane
        };

        SidebarPrivate(Sidebar * sidebar);

        Sidebar * sidebar;

        Utopia::SlideLayout * slideLayout;

        QLabel * headerLabel;
        QWidget * resultsViewWidget;
        ResultsView * resultsView;
        Utopia::ElidedLabel * searchTermLabel;
        QFrame * documentWideFrame;
        ResultsView * documentWideView;
        Utopia::WebView * webView;
        Utopia::Spinner * resultsViewSpinner;
        QLabel * listLabel;
        QComboBox * listComboBox;
        bool expectingMore;
        QLineEdit * lookupLineEdit;
        QPushButton * lookupButton;

        DocumentProxy * documentProxy;
        QMap< QString, Athenaeum::Bibliography * > citationLists;
        QStackedLayout * documentWideStackedLayout;
        Athenaeum::ArticleView * citationListView;

        void updateSpinner();

    signals:
        void termExplored(const QString & term);
        void urlRequested(const QUrl & url, const QString & target);

    public slots:
        void linkClicked(const QUrl & href, const QString & target = QString());
        void linkClickedFinished();
        void onHeaderLabelLinkActivated(const QString & link);
        void onResultsViewRunningChanged(bool running);
        void onListComboBoxCurrentIndexChanged(int index);
        void onDocumentAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void onLookupBoxActivated();
    };

}

#endif // PAPYRO_SIDEBAR_P_H
