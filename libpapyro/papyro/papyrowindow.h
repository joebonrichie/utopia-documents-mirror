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

#ifndef PAPYROWINDOW_H
#define PAPYROWINDOW_H

#include <papyro/config.h>
#include <utopia2/qt/abstractwindow.h>
#include <utopia2/busagent.h>
#include <papyro/citation.h>
#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/Document.h>
#endif
#include <string>

#include <QUrl>

class QIODevice;

namespace Utopia {
    class FileFormat;
}

namespace Utopia {
    class FlowBrowser;
}

namespace Papyro
{

    class Annotator;
    class AnnotatorRunnable;
    class PapyroTab;

    class PapyroWindowPrivate;
    class LIBPAPYRO_API PapyroWindow : public Utopia::AbstractWindow
    {
        Q_OBJECT
        U_DECLARE_PRIVATE(PapyroWindow)

    public:
        typedef enum {
            ForegroundTab,
            BackgroundTab,
            NewWindow,
            DefaultOpenTarget = ForegroundTab
        } OpenTarget;

        PapyroWindow(QWidget * parent = 0, Qt::WindowFlags f = 0);
        virtual ~PapyroWindow();

        // Type
        Utopia::Node * type() const;

        // Properties
        void clear();
        PapyroTab * currentTab() const;
        int indexOf(const QUrl & url) const;
        int indexOf(PapyroTab * tab) const;

        static void closeAll();
        static PapyroWindow * currentWindow();
        static PapyroWindow * newWindow();

    signals:
        void currentTabChanged();

    public slots:
        // Document management
        void open(Spine::DocumentHandle document, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(QIODevice * io, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(const QString & filename, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(const QUrl & url, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(const QVariantMap & citation, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(const QVariantList & citations, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(Athenaeum::CitationHandle citation, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());
        void open(QList< Athenaeum::CitationHandle > citations, OpenTarget target = DefaultOpenTarget, const QVariantMap & params = QVariantMap());

        // UI actions
        void openFile();
        void openFileFromClipboard();
        void openUrl();
        void requestUrl(const QUrl & url, const QString & target = QString());
        void saveFile();
        void saveToLibrary();
        void searchRemote(const QString & text);
        void showHelp();
        void showAbout();

        void raiseTab(int index);

    protected:
        // Special constructor
        PapyroWindow(PapyroTab * tab, QWidget * parent, Qt::WindowFlags f);

        // Dummy event
        void modelSet();

        // Events
        void closeEvent(QCloseEvent * event);
        void dragEnterEvent(QDragEnterEvent * event);
        void dragLeaveEvent(QDragLeaveEvent * event);
        void dragMoveEvent(QDragMoveEvent * event);
        void dropEvent(QDropEvent * event);
        void keyPressEvent(QKeyEvent * event);
        void keyReleaseEvent(QKeyEvent * event);
        void resizeEvent(QResizeEvent * event);
        void showEvent(QShowEvent * event);

    protected slots:
        void raiseWindow();
        void unsetModel();


    }; // class PapyroWindow




    class PapyroRecentUrlHelper : public QObject
    {
        Q_OBJECT

    public:
        PapyroRecentUrlHelper();

        static boost::shared_ptr< PapyroRecentUrlHelper > instance();

    public slots:
        void activateRecentUrl(QUrl);
        void closeAll();
    }; // class PapyroRecentUrlHelper

}

#endif // PAPYROWINDOW_H
