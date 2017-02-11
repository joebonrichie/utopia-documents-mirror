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

#include <papyro/documentfactory.h>
#include <papyro/papyrowindow.h>
#include <spine/Document.h>
#include <utopia2/global.h>
#include <utopia2/node.h>
#include <utopia2/networkaccessmanager.h>
#include <utopia2/parser.h>
#include <utopia2/initializer.h>
#include <utopia2/qt/uimanager.h>
#include <utopia2/qt/hidpi.h>
#include <utopia2/qt/preferencesdialog.h>
#include <utopia2/qt/webview.h>

#include "qtsingleapplication.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QFocusEvent>
#include <QGLFormat>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
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
//     bool notify(QObject * receiver, QEvent * e)
//     {
//         qDebug() << ">>" << receiver << e;
//         return QtSingleApplication::notify(receiver, e);
//     }

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


void print_usage(const QString & cmd) {
    QStringList lines;
    lines << QString("usage: %1 [OPTIONS] [FILE | URL ...]").arg(cmd);
    foreach (const QString & line, lines) {
        std::cout << line.toUtf8().constData() << std::endl;
    }
}

void print_help(const QString & cmd) {
    print_usage(cmd);
    QStringList lines;
    lines << QString()
          << QString("If Utopia Documents is already running, %1 will arrange for it").arg(cmd)
          << "to load an article in a new tab; otherwise it will start a new instance."
          << QString()
          << "Options:"
          << "    -h, --help          Print this message and exit."
          << "    -v, --version       Print version information and exit."
          << QString()
          << "Remaining arguments are treated as filenames or URLs of PDF articles to open."
          << QString()

#ifdef Q_OS_LINUX
          << "For more information, see the utopia-documents man page."
#endif
          << "Report bugs to <info@utopiadocs.com>";
    foreach (const QString & line, lines) {
        std::cout << line.toUtf8().constData() << std::endl;
    }
}

void suggest_help(const QString & cmd) {
    QStringList lines;
    lines << QString("Try `%1 --help' for more information.").arg(cmd);
    foreach (const QString & line, lines) {
        std::cout << line.toUtf8().constData() << std::endl;
    }
}

void print_version() {
    QStringList lines;
    lines << QString("Utopia Documents %1").arg(Utopia::versionString(true));
    foreach (const QString & line, lines) {
        std::cout << line.toUtf8().constData() << std::endl;
    }
}


int main(int argc, char *argv[])
{
    // Set up Qt
#ifdef Q_OS_WIN32
    //QApplication::setDesktopSettingsAware(false);
#endif

#ifdef Q_OS_MACX
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8) {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    }
#endif

    QStringList qargs;
    for (int i = 0; i < argc; ++i) {
        qargs << argv[i];
    }

    int opt_help = qargs.removeAll("-h") + qargs.removeAll("--help");
    int opt_version = qargs.removeAll("-v") + qargs.removeAll("--version");
    if (opt_help + opt_version > 0) {
        QString cmd(QFileInfo(qargs.first()).fileName());
        if (opt_help > 0) {
            print_help(cmd);
        } else {
            print_version();
        }
        exit(0);
    }

    // plugin paths must be changed after Application, before first GUI ops
    QDir qtDirPath(QFileInfo(argv[0]).absoluteDir());
#if defined(Q_OS_MACX)
    qtDirPath.cdUp();
    qtDirPath.cd("PlugIns");
    QCoreApplication::setLibraryPaths(QStringList(qtDirPath.canonicalPath()));
#elif defined(Q_OS_WIN)
    qtDirPath.cdUp();
    qtDirPath.cd("plugins");
    QCoreApplication::addLibraryPath(qtDirPath.canonicalPath());
#elif defined(Q_OS_LINUX)
    qtDirPath.cdUp();
    qtDirPath.cd("lib");
    qtDirPath.cd("utopia-documents");
    qtDirPath.cd("lib");
    QCoreApplication::addLibraryPath (qtDirPath.canonicalPath());
#endif

    UtopiaApplication app(argc, argv);
#ifdef Q_OS_LINUX
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path());
#elif defined(Q_OS_WIN)
    app.setWindowIcon(QIcon(":/icons/ud-logo.png"));
    QPalette p = QApplication::palette();
    p.setColor(QPalette::Active, QPalette::Highlight, QColor(174, 214, 255));
    p.setColor(QPalette::Inactive, QPalette::Highlight, QColor(220, 220, 220));
    p.setColor(QPalette::Active, QPalette::HighlightedText, QColor(0, 0, 0));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(0, 0, 0));
    QApplication::setPalette(p);
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
#if defined(Q_OS_MACX)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    app.setApplicationName("Utopia Documents");
    app.setApplicationVersion(Utopia::versionString());

    //QSettings::setDefaultFormat(QSettings::IniFormat);

    // Set user agent string for the application's networking layers
    boost::shared_ptr< Utopia::NetworkAccessManager > nam = Utopia::NetworkAccessManagerMixin().networkAccessManager();
    nam->setUserAgentString(Utopia::WebView().userAgentForUrl(QUrl()));

    // Initialise!
    Utopia::init();

    // Load in the stylesheet(s)
    {
        QFileInfoList cssFiles;
        QStringList cssSearchPaths;
        cssSearchPaths << (Utopia::resource_path() + "/css"); // Installed files
        cssSearchPaths << ":/autoload-qt"; // QResources
        QDir::setSearchPaths("css", cssSearchPaths);

        // Finding all stylesheets
        foreach (const QString & path, cssSearchPaths) {
            QDir cssDir(path);
            foreach (const QFileInfo & cssFileInfo, cssDir.entryInfoList(QStringList("*.css"), QDir::Files | QDir::Readable)) {
                cssFiles << cssFileInfo;
            }
        }

        // For each path, try to load the CSS
        QString css;
        QStringList platforms; platforms << "macosx" << "win32" << "unix";
        foreach(const QFileInfo & cssFileInfo, cssFiles) {
            // But ignore anything that looks like it's for a different platform
            QString baseName(cssFileInfo.baseName());
            if (baseName.contains("-")) {
                QString platform = baseName.section("-", -1, -1);
                if (platforms.contains(platform) && platform !=
#if defined(Q_OS_MACX)
                    "macosx"
#elif defined(Q_OS_WIN)
                    "win32"
#elif defined(Q_OS_LINUX)
                    "unix"
#endif
                    ) {
                    continue;
                }
            }
            QFile cssFile(cssFileInfo.canonicalFilePath());
            if (cssFile.open(QIODevice::ReadOnly)) {
                qDebug() << "Using stylesheet" << cssFileInfo.canonicalFilePath();
                css += cssFile.readAll();
                cssFile.close();
            }
        }

        if (!css.isEmpty()) {
			// Zoom according to hiDPI settings
			if (Utopia::hiDPIScaling() > 1.0) {
				QString newCss;
				int from = 0;
				QRegularExpressionMatch match;
				QRegularExpression regExp("(\\d+)(px)");
				while ((match = regExp.match(css, from)).hasMatch()) {
					newCss += css.mid(from, match.capturedStart(0) - from);
					int value = match.captured(1).toInt() * Utopia::hiDPIScaling();
					newCss += QString("%1").arg(value) + match.captured(2);
					from = match.capturedEnd(0);
				}
				newCss += css.mid(from);
				css = newCss;
			}

            app.setStyleSheet(css);
        }
    }

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
