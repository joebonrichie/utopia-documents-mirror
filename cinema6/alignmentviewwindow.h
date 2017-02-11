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

#ifndef ALIGNMENTVIEWWINDOW_H
#define ALIGNMENTVIEWWINDOW_H

#include <cinema6/alignmentview.h>
#include <cinema6/annotationcomponent.h>
#include <cinema6/controlaspect.h>
#include <cinema6/groupaspect.h>
#include <cinema6/keycomponent.h>
#include <cinema6/sequence.h>
#include <cinema6/sequencecomponent.h>
#include <cinema6/titleaspect.h>
#include "ui_alignmentviewwindow.h"

#include <QToolBar>

class AlignmentViewWindow : public QMainWindow, protected Ui::AlignmentViewWindow
{
    Q_OBJECT

public:
    // Constructor
    AlignmentViewWindow(QWidget * parent = 0);
    // Destructor
    ~AlignmentViewWindow();

    // Properties
    CINEMA6::AlignmentView * alignmentView();

protected:
    // Events
    void closeEvent(QCloseEvent * event);
    void resizeEvent(QResizeEvent * event);

signals:
    void forceResize();

protected:
    QToolBar * _toolBar;

}; // class AlignmentViewWindow

#endif // ALIGNMENTVIEWWINDOW_H
