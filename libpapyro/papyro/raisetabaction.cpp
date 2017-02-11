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

#include <papyro/papyrotab.h>
#include <papyro/papyrowindow.h>
#include <papyro/raisetabaction.h>
#include <papyro/raisetabaction_p.h>

#include <QEvent>
#include <QPointer>

#include <QDebug>

namespace Papyro
{

    RaiseTabActionPrivate::RaiseTabActionPrivate(RaiseTabAction * action, PapyroTab * tab)
        : QObject(action), action(action), tab(tab)
    {
        tab->installEventFilter(this);
        installOnWindow();
    }

    RaiseTabActionPrivate::~RaiseTabActionPrivate()
    {}

    void RaiseTabActionPrivate::check()
    {
        if (window && window->isActiveWindow() && window->currentTab() == tab) {
            action->setChecked(true);
        }
    }

    bool RaiseTabActionPrivate::eventFilter(QObject * obj, QEvent * e)
    {
        if (obj == tab) {
            switch (e->type()) {
            case QEvent::ParentAboutToChange:
                uninstallFromWindow();
                break;
            case QEvent::ParentChange:
                installOnWindow();
                break;
            default:
                break;
            }
        } else if (obj == window && e->type() == QEvent::WindowActivate) {
            check();
        }

        return QObject::eventFilter(obj, e);
    }

    void RaiseTabActionPrivate::installOnWindow()
    {
        if ((window = resolveWindow())) {
            window->installEventFilter(this);
            connect(window, SIGNAL(currentTabChanged()), this, SLOT(check()));
        }
    }

    void RaiseTabActionPrivate::onTabTitleChanged(const QString & title)
    {
        action->setText(title.isEmpty() ? QString("Empty Tab") : title);
    }

    void RaiseTabActionPrivate::raise()
    {
        if (window) {
            int index = window->indexOf(tab);
            if (index >= 0) {
                window->show();
                window->raise();
                window->activateWindow();
                window->raiseTab(index);
            }
        }
    }

    PapyroWindow * RaiseTabActionPrivate::resolveWindow()
    {
        // If the tab is valid, return its top-level PapyroWindow
        return tab ? qobject_cast< PapyroWindow * >(tab->window()) : 0;
    }

    void RaiseTabActionPrivate::uninstallFromWindow()
    {
        if (window) {
            window->removeEventFilter(this);
            disconnect(window, SIGNAL(currentTabChanged()), this, SLOT(check()));
            window = 0;
        }
    }

    void RaiseTabActionPrivate::update()
    {
        if (window) {
            onTabTitleChanged(tab->title());
            action->setChecked(window->isActiveWindow() && window->currentTab() == tab);
        }
    }




    RaiseTabAction::RaiseTabAction(PapyroTab * tab, QObject * parent)
        : QAction(parent), d(new RaiseTabActionPrivate(this, tab))
    {
        connect(this, SIGNAL(triggered()), d, SLOT(raise()));
        connect(tab, SIGNAL(titleChanged(const QString &)), d, SLOT(onTabTitleChanged(const QString &)));
        connect(tab, SIGNAL(destroyed()), this, SLOT(deleteLater()));
        setCheckable(true);
        d->update();
    }

    RaiseTabAction::~RaiseTabAction()
    {
        foreach (QWidget * widget, associatedWidgets()) {
            widget->removeAction(this);
        }
    }

}
