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

#ifndef ABSTRACTWINDOW_H
#define ABSTRACTWINDOW_H

#include <utopia2/qt/config.h>
#include <utopia2/extension.h>


#include <QMainWindow>
#include <QString>
#include <QWidget>
#include <QModelIndex>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMap>
#include <QUrl>
#include <boost/scoped_ptr.hpp>

class QCloseEvent;

namespace Utopia
{

    class Node;
    class AbstractWindowPrivate;
    class LIBUTOPIA_QT_API AbstractWindow : public QMainWindow
    {
        Q_OBJECT
        U_DECLARE_PRIVATE(AbstractWindow)

    public:
        typedef AbstractWindow API;

        // Constructor
        AbstractWindow(QWidget * parent = 0, Qt::WindowFlags f = 0);
        AbstractWindow(AbstractWindowPrivate * prvt, QWidget * parent = 0, Qt::WindowFlags f = 0);
        // Destructor
        ~AbstractWindow();

        // Properties
        Node * model() const;
        void setModel(Node * model);

        // Type
        virtual Node * type() const;

        // setIndex
        void setIndex(QModelIndex index);

    protected:
        void init();

        // Abstract properties
        virtual void modelSet() {};


    Q_SIGNALS:
        void requestAddToLibrary(QUrl);
        void closing(Utopia::AbstractWindow*);
        void closing(QModelIndex index);
        void fileImportAction();
        void fileImportAllAction();
        void newWindowOpened(Utopia::AbstractWindow*);
        void windowLibraryAction();

    private:
        // Model
        Node * _model;
        QModelIndex _index;

        // Top level Menus
        QMenu * _libraryMenu;
        // File Menu
        QAction * _libraryImportAction;
        QAction * _libraryImportAllAction;
        // Window menu
        QAction * _windowLibraryAction;

        // Window list
        QMap< AbstractWindow * , QAction * > _openWindows;


    protected:
        U_DEFINE_PRIVATE(AbstractWindow)

    }; // class AbstractWindow

} // namespace Utopia

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA_QT, Utopia::AbstractWindow)

#endif // ABSTRACTWINDOW_H
