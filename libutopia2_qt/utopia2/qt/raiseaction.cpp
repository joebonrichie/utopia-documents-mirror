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

#include <utopia2/qt/raiseaction.h>

#include <QEvent>
#include <QPointer>

#include <QDebug>

namespace Utopia
{

    class RaiseActionPrivate
    {
    public:
        RaiseActionPrivate(QWidget * window)
            : window(window)
        {}

        QPointer< QWidget > window;
    };




    RaiseAction::RaiseAction(QWidget * window, QObject * parent)
        : QAction(window->windowTitle(), parent), d(new RaiseActionPrivate(window))
    {
        connect(this, SIGNAL(triggered()), this, SLOT(raise()));
        setCheckable(true);
        if (d->window) {
            d->window->installEventFilter(this);
            setChecked(d->window->isActiveWindow());
            connect(d->window.data(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
        }
    }

    RaiseAction::~RaiseAction()
    {
        foreach (QWidget * widget, associatedWidgets()) {
            widget->removeAction(this);
        }
        delete d;
    }

    bool RaiseAction::eventFilter(QObject * obj, QEvent * e)
    {
        if (obj == d->window)
        {
            if (e->type() == QEvent::WindowTitleChange)
            {
                setText(d->window->windowTitle());
            }
            else if (e->type() == QEvent::WindowActivate)
            {
                setChecked(true);
            }
        }

        return QAction::eventFilter(obj, e);
    }

    void RaiseAction::raise()
    {
        if (d->window)
        {
            d->window->show();
            d->window->raise();
            d->window->activateWindow();
        }
    }

}
