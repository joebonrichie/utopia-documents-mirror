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

#include <utopia2/qt/slavesplitter.h>
#include <utopia2/qt/slavesplitter_p.h>

#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>

#include <QDebug>

namespace Utopia
{

    ///////////////////////////////////////////////////////////////////////////////////////////////

    SlaveSplitterHandle::SlaveSplitterHandle(Qt::Orientation orientation, QSplitter * parent)
        : QSplitterHandle(orientation, parent)
    {}

    SlaveSplitterHandle::~SlaveSplitterHandle()
    {}

    bool SlaveSplitterHandle::eventFilter(QObject * obj, QEvent * e)
    {
        if (QWidget * w = qobject_cast< QWidget * >(obj))
        {
            switch (e->type())
            {
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseMove:
            {
                QMouseEvent * me = static_cast< QMouseEvent * >(e);
                QMouseEvent nme(me->type(),
                                mapFromGlobal(me->globalPos()),
                                me->globalPos(),
                                me->button(),
                                me->buttons(),
                                me->modifiers());
                return QCoreApplication::instance()->notify(this, &nme);
            }
            case QEvent::Enter:
            {
                return QCoreApplication::instance()->notify(this, e);
            }
            case QEvent::HoverEnter:
            case QEvent::HoverLeave:
            case QEvent::HoverMove:
            {
                QHoverEvent * he = static_cast< QHoverEvent * >(e);
                QHoverEvent nhe(he->type(),
                                mapFromGlobal(w->mapToGlobal(he->pos())),
                                mapFromGlobal(w->mapToGlobal(he->oldPos())));
                return QCoreApplication::instance()->notify(this, &nhe);
            }
            case QEvent::Leave:
            {
                return QCoreApplication::instance()->notify(this, e);
            }
            default:
                break;
            }
        }

        return QSplitterHandle::eventFilter(obj, e);
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////

    SlaveSplitter::SlaveSplitter(QWidget * parent)
        : QSplitter(parent)
    {}

    SlaveSplitter::SlaveSplitter(Qt::Orientation orientation, QWidget * parent)
        : QSplitter(orientation, parent)
    {}

    QSplitterHandle * SlaveSplitter::createHandle()
    {
        return new SlaveSplitterHandle(orientation(), this);
    }

    void SlaveSplitter::setMaster(int index, QWidget * master)
    {
        setMaster(handle(index), master);
    }

    void SlaveSplitter::setMaster(QSplitterHandle * slave, QWidget * master)
    {
        master->installEventFilter(slave);
    }

    void SlaveSplitter::unsetMaster(int index, QWidget * master)
    {
        unsetMaster(handle(index), master);
    }

    void SlaveSplitter::unsetMaster(QSplitterHandle * slave, QWidget * master)
    {
        master->removeEventFilter(slave);
    }

}
