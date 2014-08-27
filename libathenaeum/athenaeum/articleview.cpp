/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <athenaeum/articleview.h>
#include <athenaeum/articleview_p.h>
#include <athenaeum/bibliographicmimedata_p.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QMetaMethod>
#include <QModelIndex>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QSortFilterProxyModel>

#include <QDebug>

class QMimeData;

namespace Athenaeum
{

    ArticleViewPrivate::ArticleViewPrivate(ArticleView * view)
        : QObject(view), view(view), dropping(false)
    {
        view->viewport()->installEventFilter(this);
    }

    static const QAbstractItemModel * origin(const QAbstractItemModel * model)
    {
        if (const QSortFilterProxyModel * proxy = qobject_cast< const QSortFilterProxyModel * >(model)) {
            return origin(proxy->sourceModel());
        } else {
            return model;
        }
    }

    bool ArticleViewPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        //qDebug() << "eventFilter" << obj << event;

        const AbstractBibliographicModel * model = qobject_cast< const AbstractBibliographicModel * >(view ? origin(view->model()) : 0);

        // Only filter the view's events
        if (model && obj == view->viewport()) {
            switch (event->type()) {
            case QEvent::DragMove:
            case QEvent::DragEnter: {
                QDropEvent * e = static_cast< QDropEvent * >(event);
                if ((dropping = model->acceptsDrop(e->mimeData()))) {
                    view->viewport()->update();
                    e->accept();
                } else {
                    e->ignore();
                }
                //qDebug() << "---" << dropping;
                return true;
                break;
            }
            case QEvent::DragLeave:
                dropping = false;
                view->viewport()->update();
                break;
            case QEvent::Drop:
                dropping = false;
                view->viewport()->update();
                break;
            default:
                break;
            }
        }

        return QObject::eventFilter(obj, event);
    }




    ArticleView::ArticleView(QWidget * parent)
        : QListView(parent), d(new ArticleViewPrivate(this))
    {
#ifndef Q_OS_WIN32
        QFont f(font());
        f.setPointSizeF(f.pointSizeF() * 0.85);
        setFont(f);
#endif
    }

    ArticleView::~ArticleView()
    {}

    void ArticleView::paintEvent(QPaintEvent * event)
    {
        QListView::paintEvent(event);

        if (d->dropping) {
            // Paint on top
            QPainter painter(viewport());
            painter.setRenderHint(QPainter::Antialiasing, true);
            QColor color(QColor(255, 0, 0));
            painter.setPen(QPen(color, 2));
            painter.setBrush(Qt::NoBrush);
            QRect rect(viewport()->rect().adjusted(1, 1, -1, -1));
            painter.drawRect(rect);
        }
    }

    bool ArticleView::viewportEvent(QEvent * event)
    {
        if (event->type() == QEvent::ToolTip) {
            QHelpEvent * he = static_cast< QHelpEvent * >(event);
            QModelIndex index(indexAt(he->pos()));
            if (index.isValid()) {
                emit previewRequested(index);
            }
            return true; // Don't propagate
        } else {
            return QListView::viewportEvent(event);
        }
    }

} // namespace Athenaeum
