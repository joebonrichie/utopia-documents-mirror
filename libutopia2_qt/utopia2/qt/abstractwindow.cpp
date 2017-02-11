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

#include <utopia2/qt/abstractwindow.h>
#include <utopia2/qt/abstractwindow_p.h>
#include <utopia2/qt/uimanager.h>

#include <QCloseEvent>

#include <QtDebug>

namespace Utopia
{

    AbstractWindowPrivate::AbstractWindowPrivate(AbstractWindow * publicObject)
        : QObject(), __public(publicObject)
    {
        // Synchronised menus
        menuAccount = 0; // FIXME remove this from here!

        // UI manager
        uiManager = UIManager::instance();
    }

    AbstractWindowPrivate::~AbstractWindowPrivate()
    {}

    AbstractWindow * AbstractWindowPrivate::window()
    {
        U_U(AbstractWindow);
        return u;
    }

    const AbstractWindow * AbstractWindowPrivate::window() const
    {
        U_U(const AbstractWindow);
        return u;
    }





    AbstractWindow::AbstractWindow(QWidget * parent, Qt::WindowFlags f)
        : QMainWindow(parent, f), __private(new AbstractWindowPrivate(this))
    {
        init();
    }

    AbstractWindow::AbstractWindow(AbstractWindowPrivate * prvt, QWidget * parent, Qt::WindowFlags f)
        : QMainWindow(parent, f), __private(prvt)
    {
        init();
    }

    AbstractWindow::~AbstractWindow()
    {
        U_D(AbstractWindow);
        d->uiManager->removeWindow(this);
        delete d;//->deleteLater();
    }

    void AbstractWindow::init()
    {
        U_D(AbstractWindow);
        d->uiManager->addWindow(this);

        setUnifiedTitleAndToolBarOnMac(true);
        setAttribute(Qt::WA_DeleteOnClose, true);
    }

    Node * AbstractWindow::model() const
    {
        return this->_model;
    }

    void AbstractWindow::setModel(Node * model)
    {
        // Set model
        this->_model = model;

        // Propogate event
        this->modelSet();
    }

    Node * AbstractWindow::type() const
    {
        return 0;
    }

    void AbstractWindow::setIndex(QModelIndex index)
    {
        this->_index = index;
    }

} // namespace Utopia

UTOPIA_DEFINE_EXTENSION_CLASS(Utopia::AbstractWindow)
