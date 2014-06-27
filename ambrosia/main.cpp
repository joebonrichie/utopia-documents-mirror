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

#include <QApplication>
#include <QHBoxLayout>


#include <ambrosia/ambrosiawidget.h>
#include <utopia2/global.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/initializer.h>
#include <utopia2/qt/splashscreen.h>
#include <utopia2/qt/uimanager.h>
#include <utopia2/qt/preferencesdialog.h>

#include "qtsingleapplication.h"

#include <QDir>
#include <QFileOpenEvent>
#include <QGLFormat>
#include <QTextStream>
#include <QSettings>

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
            QStringListIterator pending(_pendingFiles);
            while (pending.hasNext())
            {
                loadFile(pending.next());
            }
        }

protected:
    bool event(QEvent * event)
        {
            switch (event->type()) {
            case QEvent::FileOpen:
                if (_isReady)
                {
                    loadFile(static_cast<QFileOpenEvent *>(event)->file());
                }
                else
                {
                    _pendingFiles.append(static_cast<QFileOpenEvent *>(event)->file());
                }
                return true;
            default:
                return QtSingleApplication::event(event);
            }
        }

    void loadFile(const QString & fileName)
        {
            Utopia::UIManager::openFile(fileName);
        }

private:
    bool _isReady;
    QStringList _pendingFiles;
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

    UtopiaApplication app(argc, argv);
#ifdef Q_OS_LINUX
#   if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path().string().c_str());
#   else
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path().file_string().c_str());
#   endif
#endif

    // What documents should this application open?
    QString command;
    QStringList documents;
    QTextStream commandStream(&command);
    commandStream << QString("open");
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i] && argv[i][0] != '-')
        {
            commandStream << "|" << QString(argv[i]);
            documents << QString(argv[i]);
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
    app.setApplicationName("Utopia Molecules");

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
        bases << QFileInfo(QCoreApplication::applicationFilePath()).baseName().toLower();
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
                QFile styleFile((Utopia::resource_path() / "css" / (base + modifier + ".css").toAscii().constData()).file_string().c_str());
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
    Utopia::SplashScreen splash;
    splash.show();
    app.flush();
    Utopia::init(&splash);
    splash.hide();

    Utopia::Node * authority = 0;
    Utopia::Node * model = 0;

    if (argc == 2)
    {
        // Parse!
        Utopia::Parser::Context ctx = Utopia::load(argv[1], Utopia::FileFormat::get("PDB"));

        // Check for error
        if (ctx.errorCode() != Utopia::Parser::None)
        {
            qCritical() << "Error:" << ctx.message();
        }
        QListIterator< Utopia::Parser::Warning > warnings(ctx.warnings());
        while (warnings.hasNext())
        {
            Utopia::Parser::Warning warning = warnings.next();
            qCritical() << "Warning:" << warning.message;
            if (warning.line > 0)
            {
                qCritical() << "  @line:" << warning.line;
            }
        }

        // Delete model
        if ((authority = ctx.model()))
        {
            Utopia::Node::relation::iterator doc = ctx.model()->relations(Utopia::UtopiaSystem.hasPart).begin();
            Utopia::Node::relation::iterator end = ctx.model()->relations(Utopia::UtopiaSystem.hasPart).end();

            if (doc != end)
            {
                model = *doc;
            }
            else
            {
                qDebug() << "No model found in ctx.model()";
            }
        }
    }

    QWidget w;
    AMBROSIA::AmbrosiaWidget * window = new AMBROSIA::AmbrosiaWidget;
    QHBoxLayout * layout = new QHBoxLayout(&w);
    layout->addWidget(window);
    layout->setContentsMargins(0, 0, 0, 0);
    w.show();
    if (model) { window->load(model); }

    app.makeReady();

    int result = app.exec();

    //serviceManager.saveToSettings();

    if (authority) { delete authority; }

    Utopia::Initializer::cleanup();

    return result;
}
