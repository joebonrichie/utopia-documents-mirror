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

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <utopia2/qt/config.h>

#include <boost/shared_ptr.hpp>

#include <QList>
#include <QMenuBar>
#include <QObject>
#include <QUrl>

class QAction;
class QActionGroup;

namespace Utopia
{

    class PreferencesDialog;
    class AbstractWindow;
    class AbstractWindowPrivate;

    class UIManagerPrivate;
    class UIManager : public QObject
    {
        Q_OBJECT

    public:
        ~UIManager();

        // Registration of UI
        void addRecentFile(QUrl url);
        void addWindow(AbstractWindow * window);
        void removeWindow(AbstractWindow * window);

        // Public methods
        static void openFile(const QString & fileName);
        static void openUrl(const QUrl & url);

        static boost::shared_ptr< UIManager > instance();

        QList< AbstractWindow * > windows() const;
        template< class T >
        QList< T * > windows() const
        {
            QList< T * > matches;
            foreach (AbstractWindow * window, windows()) {
                if (T * match = dynamic_cast< T * >(window)) {
                    matches.append(match);
                }
            }
            return matches;
        }

        QMenu * menuRecent(QWidget * parent = 0) const;
        QMenu * menuWindow(QWidget * parent = 0) const;
        QMenu * menuWindowMaster() const;
        QActionGroup * menuWindowActionGroup() const;
        QMenu * menuHelp(QWidget * parent = 0) const;
        QAction * actionPreferences() const;

    public slots:
        void clearRecentUrls();
        void libraryWasClosed();
        void onMessage(const QString & message);
        void relayRecentUrlActivation();
        void showPreferences(const QString & paneName = QString(), const QVariant & params = QVariant());
        int windowCount();

        void showAbout();
        void showHelp();

    signals:
        void activateRecentUrl(QUrl url);
        void createAccount();

    protected:
        UIManager();
        UIManagerPrivate * d;
    };

}

#endif // UIMANAGER_H
