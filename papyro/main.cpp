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

#include <papyro/documentfactory.h>
#include <papyro/papyrowindow.h>
#include <spine/Document.h>
#include <utopia2/global.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/initializer.h>
#include <utopia2/qt/uimanager.h>
#include <utopia2/qt/preferencesdialog.h>

#include "qtsingleapplication.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QFocusEvent>
#include <QGLFormat>
#include <QSettings>
#include <QTextStream>

#include <QDebug>
#include <QEvent>

class UtopiaApplication : public QtSingleApplication
{
public:
    UtopiaApplication(int & argc, char ** argv)
        : QtSingleApplication(argc, argv), _isReady(false)
    {}

    void makeReady()
    {
        _isReady = true;
        QListIterator< QUrl > pending(_pendingFiles);
        while (pending.hasNext())
        {
            loadUrl(pending.next());
        }
    }

protected:
    bool event(QEvent * event)
    {
        switch (event->type()) {
        case QEvent::FileOpen: {
            // This step is to make sure an encoded URL isn't re-encoded on OS X
            QUrl url = QUrl::fromEncoded(static_cast<QFileOpenEvent *>(event)->url().toString().toUtf8());
            if (_isReady) {
                loadUrl(url);
            } else {
                _pendingFiles.append(url);
            }
            return true;
        }
        default:
            return QtSingleApplication::event(event);
        }
    }

    void loadUrl(const QUrl & url)
    {
        Utopia::UIManager::openUrl(url);
    }

private:
    bool _isReady;
    QList< QUrl > _pendingFiles;
};



int main(int argc, char *argv[])
{
    // Set up Qt
#ifdef Q_OS_LINUX
    QApplication::setGraphicsSystem("raster");
#endif
    //QApplication::setGraphicsSystem("raster");
    for (int i = 0; i < argc; ++i)
    {
        qDebug() << "*** ARG" << argv[i];
    }

#ifdef Q_OS_WIN32
    QApplication::setDesktopSettingsAware(false);
#endif

#ifdef Q_OS_MACX
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

    UtopiaApplication app(argc, argv);
#ifdef Q_OS_LINUX
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path());
#endif

    // What documents should this application open?
    QString command;
    QStringList documents;
    QTextStream commandStream(&command);
    commandStream << QString("open");
	QStringList args(QCoreApplication::arguments());
	args.pop_front();
    foreach (QString arg, args) {
        if (!arg.isEmpty() && arg[0] != '-') {
            commandStream << "|" << QString(arg);
            documents << QString(arg);
        }
    }
    commandStream.flush();

    // Only allow *ONE* instance of the application to exist at any one time.
    qDebug() << "*** COMMAND" << command;
    if (app.sendMessage(command))
    {
        return 0;
    }

    // Set OpenGL global settings
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(4);
    QGLFormat::setDefaultFormat(glf);

    app.setAttribute(Qt::AA_DontShowIconsInMenus);
    app.setApplicationName("Utopia Library");

    // plugin paths must be changed after Application, before first GUI ops
    QDir qtDirPath(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MACX)
    qtDirPath.cdUp();
    qtDirPath.cd("PlugIns");
    QCoreApplication::setLibraryPaths(QStringList(qtDirPath.canonicalPath()));
#elif defined(Q_OS_WIN32)
    qtDirPath.cdUp();
    qtDirPath.cd("plugins");
    QCoreApplication::addLibraryPath(qtDirPath.canonicalPath());
    QPalette p = QApplication::palette();
    p.setColor(QPalette::Active, QPalette::Highlight, QColor(174, 214, 255));
    p.setColor(QPalette::Inactive, QPalette::Highlight, QColor(220, 220, 220));
    p.setColor(QPalette::Active, QPalette::HighlightedText, QColor(0, 0, 0));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(0, 0, 0));
    QApplication::setPalette(p);
    app.setWindowIcon(QIcon(":/icons/ud-logo.png"));
#elif defined(Q_OS_LINUX)
    qtDirPath.cdUp();
    qtDirPath.cd("lib");
    QCoreApplication::addLibraryPath (qtDirPath.canonicalPath());
#endif

    // Load in the stylesheet(s)
    {
        QStringList bases;
        bases << "utopia";
        bases << QFileInfo(QCoreApplication::applicationFilePath()).baseName().toLower().section(QRegExp("[^\\w]+"), -1, -1, QString::SectionSkipEmpty);
        QStringList modifiers;
        modifiers << "";
#if defined(Q_OS_MACX)
        modifiers << "-macosx";
#elif defined(Q_OS_WIN32)
        modifiers << "-win32";
#elif defined(Q_OS_LINUX)
        modifiers << "-unix";
#endif
        QString css;
        QStringListIterator base_iter(bases);
        while (base_iter.hasNext())
        {
            QString base = base_iter.next();
            QStringListIterator modifier_iter(modifiers);
            while (modifier_iter.hasNext())
            {
                QString modifier = modifier_iter.next();
                QFile styleFile(Utopia::resource_path() + "/css/" + base + modifier + ".css");
                if (styleFile.exists())
                {
                    qDebug() << "Using stylesheet:" << styleFile.fileName();
                    styleFile.open(QIODevice::ReadOnly);
                    css += styleFile.readAll();
                    styleFile.close();
                }
            }
        }
        if (!css.isEmpty())
        {
            app.setStyleSheet(css);
        }
    }

    // Initialise!
    Utopia::init();

    //Utopia::PreferencesDialog dialog;
    //dialog.show();

    boost::shared_ptr< Utopia::UIManager > uiManager(Utopia::UIManager::instance());
    {
        QObject::connect(&app, SIGNAL(messageReceived(const QString &)), uiManager.get(), SLOT(onMessage(const QString &)));

        Papyro::PapyroWindow * window = new Papyro::PapyroWindow;
        window->show();
        window->raise();

        if (documents.size() > 0)
        {
            uiManager->onMessage(command);
        }

        // Set CWD
        QDir::setCurrent(QDir::homePath());
    }

    app.makeReady();
#if defined(Q_OS_MACX)
    //app.setQuitOnLastWindowClosed(false);
#endif

    int result = app.exec();

    Utopia::Initializer::cleanup();

    return result;
}
