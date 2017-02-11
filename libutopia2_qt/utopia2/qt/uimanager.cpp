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

#include "uimanager_p.h"
#include "uimanager.h"

#include <utopia2/qt/aboutdialog.h>
#include <utopia2/qt/abstractwindow.h>
#include <utopia2/qt/abstractwindow_p.h>
#include <utopia2/qt/preferencesdialog.h>
#include <utopia2/qt/raiseaction.h>
#include <utopia2/qt/slavemenu.h>

#include <boost/weak_ptr.hpp>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QSettings>
#include <QStringList>
#include <QUrl>

#include <QDebug>

namespace Utopia
{

    /// UIManagerPrivate ////////////////////////////////////////////////////////////////

    UIManagerPrivate::UIManagerPrivate(QObject * parent)
        : QObject(parent)
    {}

    UIManagerPrivate::~UIManagerPrivate()
    {}




    /// UIManager ///////////////////////////////////////////////////////////////////////


    UIManager::UIManager()
        : QObject(), d(new UIManagerPrivate(this))
    {
        d->preferencesDialog = PreferencesDialog::instance();

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Shared menus and actions

        d->actionClearHistory = new QAction("Clear History", this);
        connect(d->actionClearHistory, SIGNAL(triggered()), this, SLOT(clearRecentUrls()));

        d->actionPreferences = new QAction("Preferences", this);
        d->actionPreferences->setShortcut(QKeySequence::Preferences);
        d->actionPreferences->setMenuRole(QAction::PreferencesRole);
        connect(d->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));

        d->windowActionGroup = new QActionGroup(this);
        d->windowActionGroup->setExclusive(true);

        d->menuRecent.setTitle("Recent Files");
        d->menuRecent.setEnabled(false);
        d->menuRecent.addSeparator();
        d->menuRecent.addAction(d->actionClearHistory);

        d->menuWindow.setTitle("&Window");

        d->menuHelp.setTitle("Help");
        d->menuHelp.addAction("View Quick Start Guide", this, SLOT(showHelp()));
        d->menuHelp.addAction("About Utopia Documents", this, SLOT(showAbout()));


        ///////////////////////////////////////////////////////////////////////////////////////////
        // Populate recent files list

        // Settings
        QSettings settings;
        settings.beginGroup("/Common");
        QStringList urls(settings.value("/Recent Files").toStringList());
        int maxRecentUrls(qMax(10, settings.value("/Maximum Recent Files Count", 30).toInt()));
        int urlsToSkip = qMax(0, urls.size() - maxRecentUrls);
        QStringListIterator recentUrls(urls);
        while (recentUrls.hasNext()) {
            QUrl url(recentUrls.next());

            if (urlsToSkip-- > 0) { continue; }

            // Ignore non-existant files
            if (url.isLocalFile() && QFileInfo(url.toLocalFile()).exists()) {
                addRecentFile(url);
            }
        }
        settings.endGroup();

    }

    UIManager::~UIManager()
    {
        // Settings
        QSettings settings;
        settings.beginGroup("/Common");

        // Save recent files
        QStringList urls;
        QListIterator< QUrl > urlIter(d->recentUrls);
        urlIter.toBack();
        int maxRecentUrls(qMax(10, settings.value("/Maximum Recent Files Count", 30).toInt()));
        int urlsToSkip = qMax(0, d->recentUrls.size() - maxRecentUrls);
        while (urlIter.hasPrevious())
        {
            if (urlsToSkip-- > 0) { continue; }

            urls << urlIter.previous().toString();
        }
        urls.removeDuplicates();
        settings.setValue("/Recent Files", urls);

        settings.endGroup();

        //qDebug() << "<<<<<<<<<<<" << "UIManager::~UIManager()";
    }

    QAction * UIManager::actionPreferences() const
    {
        return d->actionPreferences;
    }

    void UIManager::addRecentFile(QUrl url)
    {
        d->menuRecent.setEnabled(true);

        // Remove all old URL openers with this URL and any excess entries
        QList< QAction * > firingLine(d->menuRecent.actions());
        QListIterator< QAction * > firingLineIter(firingLine);
        while (firingLineIter.hasNext()) {
            QAction * potential = firingLineIter.next();
            if (potential->data().toUrl() == url) {
                d->menuRecent.removeAction(potential);
                //qDebug() << "deleting1" << potential;
                delete potential;
            }
        }

        QSettings settings;
        settings.beginGroup("/Common");
        int maxRecentUrls(qMax(10, settings.value("/Maximum Recent Files Count", 30).toInt()));
        firingLine = d->menuRecent.actions();
        firingLineIter = firingLine;
        while (firingLineIter.hasNext()) {
            QAction * potential = firingLineIter.next();
            if (potential->data().isValid() && --maxRecentUrls < 0) {
                d->menuRecent.removeAction(potential);
                //qDebug() << "deleting2" << potential;
                delete potential;
            }
        }

        // Create new URL opener action
        QString name;
        if (url.scheme() == "file")
        {
            QFileInfo fileInfo(url.toLocalFile());
            name = fileInfo.fileName() + " - " + fileInfo.dir().canonicalPath();
        }
        else
        {
            name = url.toString();
        }
        QAction * openUrlAction = new QAction(name, this);
        openUrlAction->setData(url);
        connect(openUrlAction, SIGNAL(triggered()), this, SLOT(relayRecentUrlActivation()));

        // Add new URL opener action
        //qDebug() << "adding for new document...";
        d->menuRecent.insertAction(d->menuRecent.actions().size() == 0 ? 0 : d->menuRecent.actions().front(), openUrlAction);

        // Keep list up-to-date
        d->recentUrls.removeAll(url);
        d->recentUrls.prepend(url);
    }

    void UIManager::addWindow(AbstractWindow * window)
    {
        // Add window to registry
        d->windows.append(window);

        // Add new raise action to all window menus
        //QAction * raiseAction = new Utopia::RaiseAction(window);
        //raiseAction->setActionGroup(d->windowActionGroup);
        //d->menuWindow.addAction(raiseAction);
    }

    void UIManager::clearRecentUrls()
    {
        // Clear all URL links
        QList< QAction * > firingLine(d->menuRecent.actions());
        QListIterator< QAction * > firingLineIter(firingLine);
        while (firingLineIter.hasNext()) {
            QAction * potential = firingLineIter.next();
            if (potential->data().isValid()) {
                d->menuRecent.removeAction(potential);
                //qDebug() << "deleting" << potential;
                delete potential;
            }
        }

        // Disable menu
        d->menuRecent.setEnabled(false);

        d->recentUrls.clear();
    }

    boost::shared_ptr< UIManager > UIManager::instance()
    {
        static boost::weak_ptr< UIManager > singleton;
        boost::shared_ptr< UIManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< UIManager >(new UIManager());
            singleton = shared;
        }
        return shared;
    }

    void UIManager::libraryWasClosed()
    {
        // If there is only Library left and it's been closed, exit
        qDebug() << "libraryWasClosed()" << d->windows.count();
        if (d->windows.count() == 1)
        {
            QApplication * app = (QApplication * ) QApplication::instance();
            app->closeAllWindows();
            app->quit();
        }
    }

    QMenu * UIManager::menuRecent(QWidget * parent) const
    {
        return SlaveMenu::slave(&d->menuRecent, parent);
    }

    QMenu * UIManager::menuWindow(QWidget * parent) const
    {
        return SlaveMenu::slave(&d->menuWindow, parent);
    }

    QMenu * UIManager::menuWindowMaster() const
    {
        return &d->menuWindow;
    }

    QActionGroup * UIManager::menuWindowActionGroup() const
    {
        return d->windowActionGroup;
    }

    QMenu * UIManager::menuHelp(QWidget * parent) const
    {
        return SlaveMenu::slave(&d->menuHelp, parent);
    }

    void UIManager::onMessage(const QString & message)
    {
        QStringList tokens(message.split("|"));
        if (!tokens.isEmpty()) {
            QString command(tokens.takeFirst());
            while (!tokens.isEmpty()) {
                QString token(tokens.takeFirst());
                QUrl url;
                if (token.contains("://")) {
                    url = QUrl::fromEncoded(token.toUtf8());
                }
                if (url.scheme().isEmpty()) {
                    url = QUrl::fromLocalFile(token);
                }
                openUrl(url);
            }
        }
    }

    void UIManager::openFile(const QString & fileName)
    {
        openUrl(QUrl::fromLocalFile(fileName));
    }

    void UIManager::openUrl(const QUrl & url)
    {
        emit instance()->activateRecentUrl(url);
    }

    void UIManager::relayRecentUrlActivation()
    {
        QAction * action = qobject_cast< QAction * >(sender());
        emit activateRecentUrl(action->data().toUrl());
    }

    void UIManager::removeWindow(AbstractWindow * window)
    {
        d->windows.removeAll(window);
    }

    void UIManager::showAbout()
    {
        AboutDialog * about = new AboutDialog();
        about->setWindowModality(Qt::ApplicationModal);
        about->show();
        about->raise();
    }

    void UIManager::showHelp()
    {
        QDesktopServices::openUrl(QUrl("http://utopiadocs.com/redirect.php?to=quickstart"));
    }

    void UIManager::showPreferences(const QString & paneName, const QVariant & params)
    {
        d->preferencesDialog->focusPane(paneName, params);
        d->preferencesDialog->show();
        d->preferencesDialog->raise();
    }

    QList< AbstractWindow * > UIManager::windows() const
    {
        return d->windows;
    }

    int UIManager::windowCount()
    {
        return d->windows.count();
    }
}
