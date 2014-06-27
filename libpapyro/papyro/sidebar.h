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

#ifndef PAPYRO_SIDEBAR_H
#define PAPYRO_SIDEBAR_H

#include <QFrame>
#include <QUrl>

namespace Utopia
{
    class WebView;
}

namespace Papyro
{

    class ResultsView;

    class SidebarPrivate;
    class Sidebar : public QFrame
    {
        Q_OBJECT
        Q_ENUMS(SidebarMode)

    public:
        enum SidebarMode { DocumentWide, Results, Web };

        Sidebar(QWidget * parent = 0);

        ResultsView * documentWideView() const;
        ResultsView * resultsView() const;
        void setSearchTerm(const QString & term);
        void setMode(SidebarMode mode);
        Utopia::WebView * webView() const;

    signals:
        void selectionChanged();
        void urlRequested(const QUrl & url, const QString & target);

    public slots:
        void clear();
        void copySelectedText();
        void lookupStarted();
        void lookupStopped();
        void onSelectionChanged();

    private:
        SidebarPrivate * d;
    };

}

#endif // PAPYRO_SIDEBAR_H
