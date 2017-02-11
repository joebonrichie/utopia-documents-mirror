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

#include <utopia2/ontology.h>
#include <utopia2/global.h>
#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/library.h>
#include <utopia2/qt/splashscreen.h>
#include <utopia2/qt/filedialog.h>
#include <utopia2/qt/filefixerdialog.h>

#include "alignmentviewwindow.h"

#include <QApplication>
#include <QDir>

#include <QDebug>
#include <QEvent>


int main(int argc, char *argv[])
{
    // Set up Qt
#ifdef Q_OS_LINUX
    QApplication::setGraphicsSystem("raster");
#   if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path().string().c_str());
#   else
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, Utopia::resource_path().file_string().c_str());
#   endif
#endif
    //QApplication::setGraphicsSystem("raster");
    for (int i = 0; i < argc; ++i)
    {
        qDebug() << "*** ARG" << argv[i];
    }

#ifdef Q_OS_WIN32
    QApplication::setDesktopSettingsAware(false);

#endif

    QApplication app(argc, argv);

    app.setAttribute(Qt::AA_DontShowIconsInMenus);
    app.setApplicationName("Utopia Sequences");

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
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                QFile styleFile((Utopia::resource_path() / "css" / (base + modifier + ".css").toAscii().constData()).string().c_str());
#else
                QFile styleFile(Utopia::resource_path() + "css" +  base + modifier + ".css");
#endif
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

    QString filename;
    Utopia::FileFormat * format = 0;
    if (argc != 2)
    {
        QPair< QString, Utopia::FileFormat * > toOpen = getOpenFileName(0, "Load Sequences", QString(), Utopia::SequenceFormat);
        filename = toOpen.first;
        format = toOpen.second;
    }
    else
    {
        filename = argv[1];
    }

    if (filename.isEmpty())
    {
        exit(0);
    }

    // Parse!
    Utopia::Parser::Context ctx = Utopia::load(filename, format ? format : Utopia::FileFormat::get("PIR"));

    // Check for error
    if (ctx.errorCode() != Utopia::Parser::None)
    {
        Utopia::FileFixerDialog dialog(ctx, filename, format);

        dialog.show();
        if (dialog.exec() == QDialog::Accepted)
        {
            ctx = dialog.ctx();
        }
        else
        {
            qCritical() << "Error:" << ctx.message();
        }
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
    if (ctx.model())
    {
        Utopia::Node::relation::iterator seq = ctx.model()->relations(Utopia::UtopiaSystem.hasPart).begin();
        Utopia::Node::relation::iterator end = ctx.model()->relations(Utopia::UtopiaSystem.hasPart).end();
        
        if (seq != end)
        {
            // Widget
            AlignmentViewWindow * window = new AlignmentViewWindow;
            app.setQuitOnLastWindowClosed(true);

            Utopia::Node* p_title = Utopia::UtopiaDomain.term("title");

            CINEMA6::AlignmentView * av = window->alignmentView();
            av->appendComponent(CINEMA6::AlignmentView::Top, new CINEMA6::KeyComponent());

            for (; seq != end; ++seq)
            {
                Utopia::Node::relation::iterator seq2 = (*seq)->relations(Utopia::UtopiaSystem.hasPart).begin();
                Utopia::Node::relation::iterator end2 = (*seq)->relations(Utopia::UtopiaSystem.hasPart).end();
                if (seq2 != end2)
                {
                    qDebug() << "adding sequence" << (*seq2)->attributes.get(p_title).toString();
                    av->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::SequenceComponent(new CINEMA6::Sequence(*seq2)));
                
                }
            }

//             av->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::SequenceComponent(*seq, "OPSD_SHEEP"));
//            CINEMA6::AnnotationComponent* ann =
//                    new CINEMA6::AnnotationComponent("Hydrophobicity");
            //           av->appendComponent(CINEMA6::AlignmentView::Center, ann);
//            av->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::KeyComponent());
//             av->appendComponent(CINEMA6::AlignmentView::Center, new CINEMA6::SequenceComponent(*seq, "OPSD_BOVIN"));
            av->appendComponent(CINEMA6::AlignmentView::Bottom, new CINEMA6::KeyComponent());
//            av->appendAspect(CINEMA6::AlignmentView::Left, new CINEMA6::GroupAspect("Group Control"));
            av->appendAspect(CINEMA6::AlignmentView::Left, new CINEMA6::TitleAspect("Names"));
            av->appendAspect(CINEMA6::AlignmentView::Right, new CINEMA6::ControlAspect("Control"));
            //ann->setHeightFactor(4);

            window->show();

            // Run Qt
            app.exec();
        }
        else
        {
            qDebug() << "No sequence found in ctx.model()";
        }

        delete ctx.model();
    }

    return 0;
}
