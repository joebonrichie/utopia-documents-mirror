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

#include "alignmentviewwindow.h"

#include <QCloseEvent>

AlignmentViewWindow::AlignmentViewWindow(QWidget * parent)
    : QMainWindow(parent, Qt::Window)
{
    // Set up UI
    this->setupUi(this);

    // Set Mac HIToolbar
    this->setUnifiedTitleAndToolBarOnMac(true);
    this->_toolBar = new QToolBar(this);
    this->addToolBar(this->_toolBar);
    QListIterator< QAction * > viewActions(this->alignmentView()->actions());
    while (viewActions.hasNext())
    {
        QAction * action = viewActions.next();
        this->_toolBar->addAction(action);
    }

    // Set up widget modes
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setWindowModified(true);

#ifdef Q_WS_MAC
    // Required on OS X due to a bug in handling of unified toolbars
//    connect(this, SIGNAL(forceResize()), this->toolBar, SLOT(forceResize()));
#endif

    // Initialise the tool bar
//    toolBar->addSearchItemsAndInitialise();
}

AlignmentViewWindow::~AlignmentViewWindow()
{
}

CINEMA6::AlignmentView * AlignmentViewWindow::alignmentView()
{
    return this->_alignmentView;
}

void AlignmentViewWindow::closeEvent(QCloseEvent * event)
{
    event->accept();
}

void AlignmentViewWindow::resizeEvent(QResizeEvent * event)
{
    emit forceResize();
    QMainWindow::resizeEvent(event);
}
