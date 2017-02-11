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

#ifndef UTOPIA_QT_PLUGINSPREFERENCESPANE_P_H
#define UTOPIA_QT_PLUGINSPREFERENCESPANE_P_H

#include <QObject>
#include <QList>

class QListWidget;
class QWebElement;

namespace Utopia
{
    class Configurator;
    class WebPage;
    class WebView;
} // namespace Utopia




class ConfiguratorControl : public QObject
{
    Q_OBJECT

public:
    ConfiguratorControl(Utopia::Configurator * configurator,
                        QObject * parent);

    Utopia::Configurator * configurator;
    Utopia::WebPage * page;
    bool blockSignals;

    Q_INVOKABLE void revert();
    void revert(QWebElement & inputElement);
    void save();

signals:
    void contentsChanged();

public slots:
    void onConfigurationChanged(const QString & key);
    void onWebPageContentsChanged();
    void onWebPageLinkClicked(const QUrl &);

}; // class ConfiguratorControl;




class PluginsPreferencesPanePrivate : public QObject
{
    Q_OBJECT

public:
    PluginsPreferencesPanePrivate(QObject * parent = 0);

    QList< ConfiguratorControl * > configurators;
    QListWidget * listWidget;
    Utopia::WebView * webView;
}; // class PluginsPreferencesPanePrivate


#endif // UTOPIA_QT_PLUGINSPREFERENCESPANE_P_H
