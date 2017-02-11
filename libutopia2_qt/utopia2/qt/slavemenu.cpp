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

#include <utopia2/qt/slavemenu.h>

#include <QActionEvent>
#include <QPointer>

#include <QDebug>

namespace Utopia
{

    class SlaveMenuPrivate
    {
    public:
        SlaveMenuPrivate() : stack(0) {}

        QPointer< QMenu > master;
        int stack;
    };




    SlaveMenu::SlaveMenu(QWidget * parent)
        : QMenu(parent), d(new SlaveMenuPrivate)
    {}

    SlaveMenu::SlaveMenu(const QString & title, QWidget * parent)
        : QMenu(title, parent), d(new SlaveMenuPrivate)
    {}

    SlaveMenu::~SlaveMenu()
    {}

    void SlaveMenu::actionEvent(QActionEvent * event) {
        // If an action has been added or removed, mirror on the master
        if (d->stack == 0 && d->master) {
            ++d->stack;

            switch (event->type()) {
            case QEvent::ActionAdded:
                d->master->insertAction(event->before(), event->action());
                break;
            case QEvent::ActionRemoved:
                d->master->removeAction(event->action());
                break;
            case QEvent::ActionChanged:
            default:
                // Nothing to do here
                break;
            }

            --d->stack;
        }
        QMenu::actionEvent(event);
    }

    bool SlaveMenu::eventFilter(QObject * obj, QEvent * event)
    {
        if (d->stack == 0 && d->master && d->master == obj) {
            ++d->stack;

            switch (event->type()) {
            case QEvent::ActionAdded: {
                QActionEvent * actionEvent = dynamic_cast< QActionEvent * >(event);
                insertAction(actionEvent->before(), actionEvent->action());
                break;
            }
            case QEvent::ActionRemoved: {
                QActionEvent * actionEvent = dynamic_cast< QActionEvent * >(event);
                removeAction(actionEvent->action());
                break;
            }
            case QEvent::EnabledChange: {
                setEnabled(d->master->isEnabled());
                break;
            }
            case QEvent::ActionChanged:
            default:
                // Nothing to do here
                break;
            }

            --d->stack;
        }

        return QMenu::eventFilter(obj, event);
    }

    QMenu * SlaveMenu::master() const
    {
        return d->master;
    }

    void SlaveMenu::setMaster(QMenu * menu)
    {
        if (d->master) {
            d->master->removeEventFilter(this);
            setTitle(QString());
            clear();
        }
        d->master = menu;
        if (d->master) {
            d->master->installEventFilter(this);
            setTitle(d->master->title());
            setEnabled(d->master->isEnabled());
            ++d->stack;
            addActions(d->master->actions());
            --d->stack;
        }
    }

    SlaveMenu * SlaveMenu::slave(QMenu * master, QWidget * parent)
    {
        SlaveMenu * menu = new SlaveMenu(parent);
        menu->setMaster(master);
        return menu;
    }

}
