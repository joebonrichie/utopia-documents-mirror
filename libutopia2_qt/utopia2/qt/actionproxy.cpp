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

#include <utopia2/qt/actionproxy.h>
#include <utopia2/qt/actionproxy_p.h>
#include <utopia2/qt/menuproxy.h>

#include <QMenu>

#include <QDebug>

namespace Utopia
{

    ActionProxyPrivate::ActionProxyPrivate(ActionProxy * proxy)
        : QObject(proxy), proxy(proxy), recursed(false), proxiedMenu(0)
    {
        proxy->setEnabled(false);
    }

    void ActionProxyPrivate::onProxiedActionChanged()
    {
        if (proxied && !recursed) {
            recursed = true;
            proxy->setAutoRepeat(proxied->autoRepeat());
            proxy->setCheckable(proxied->isCheckable());
            proxy->setEnabled(proxied->isEnabled());
            proxy->setFont(proxied->font());
            proxy->setIcon(proxied->icon());
            proxy->setIconText(proxied->iconText());
            proxy->setIconVisibleInMenu(proxied->isIconVisibleInMenu());
            proxy->setMenuRole(proxied->menuRole());
            proxy->setPriority(proxied->priority());
            proxy->setShortcut(proxied->shortcut());
            proxy->setShortcutContext(proxied->shortcutContext());
            proxy->setStatusTip(proxied->statusTip());
            proxy->setText(proxied->text());
            proxy->setToolTip(proxied->toolTip());
            proxy->setVisible(proxied->isVisible());
            proxy->setWhatsThis(proxied->whatsThis());
            recursed = false;
        }
    }

    void ActionProxyPrivate::onProxiedActionToggled(bool checked)
    {
        if (proxied && !recursed) {
            recursed = true;
            proxy->setChecked(checked);
            recursed = false;
        }
    }




    ActionProxy::ActionProxy(QObject * parent)
        : QAction(parent), d(new ActionProxyPrivate(this))
    {}

    ActionProxy::ActionProxy(const QString & text, QObject * parent)
        : QAction(text, parent), d(new ActionProxyPrivate(this))
    {}

    ActionProxy::ActionProxy(QAction * proxied, QObject * parent)
        : QAction(parent), d(new ActionProxyPrivate(this))
    {
        setProxied(proxied);
    }

    void ActionProxy::setProxied(QAction * proxied)
    {
        if (d->proxied) {
            // Unhook old proxied action
        }
        d->proxied = proxied;
        if (proxied) {
            d->onProxiedActionChanged();

            if (d->proxied->menu()) {
                if (d->proxiedMenu) {
                    d->proxiedMenu->deleteLater();
                }
                d->proxy->setMenu(d->proxiedMenu = new MenuProxy(d->proxied->menu()));
            }

            // Hook up new proxied action
            connect(d->proxy, SIGNAL(hovered()), d->proxied, SLOT(hover()));
            connect(d->proxy, SIGNAL(triggered()), d->proxied, SLOT(trigger()));

            connect(d->proxied, SIGNAL(changed()), d, SLOT(onProxiedActionChanged()));
            connect(d->proxied, SIGNAL(toggled(bool)), d, SLOT(onProxiedActionToggled(bool)));
        }
    }

} // namespace Utopia