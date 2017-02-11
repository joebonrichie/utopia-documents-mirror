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

#include <utopia2/qt/menuproxy.h>
#include <utopia2/qt/menuproxy_p.h>

#include <QActionEvent>
#include <QMenu>

#include <QDebug>

namespace Utopia
{

    MenuProxyPrivate::MenuProxyPrivate(MenuProxy * proxy)
        : QObject(proxy), proxy(proxy), recursed(false)
    {
        proxy->setEnabled(false);
    }

    bool MenuProxyPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        if (proxied && proxied == obj) {
            QActionEvent * ae = dynamic_cast< QActionEvent * >(event);
            switch (event->type()) {
            case QEvent::ActionAdded:
                proxy->insertAction(ae->before(), ae->action());
                break;
            case QEvent::ActionRemoved:
                proxy->removeAction(ae->action());
                break;
            case QEvent::EnabledChange:
                proxy->setEnabled(proxied->isEnabled());
                break;
            default:
                break;
            }
        }

        return QObject::eventFilter(obj, event);
    }




    MenuProxy::MenuProxy(QWidget * parent)
        : QMenu(parent), d(new MenuProxyPrivate(this))
    {}

    MenuProxy::MenuProxy(const QString & text, QWidget * parent)
        : QMenu(text, parent), d(new MenuProxyPrivate(this))
    {}

    MenuProxy::MenuProxy(QMenu * proxied, QWidget * parent)
        : QMenu(parent), d(new MenuProxyPrivate(this))
    {
        setProxied(proxied);
    }

    void MenuProxy::setProxied(QMenu * proxied)
    {
        if (d->proxied) {
            // Unhook old proxied menu
        }
        d->proxied = proxied;
        if (proxied) {
            // Copy current menu structure
            addActions(d->proxied->actions());
            // Ensure it's got the same enablement
            setEnabled(d->proxied->isEnabled());

            d->proxied->installEventFilter(d);
        }
    }

} // namespace Utopia