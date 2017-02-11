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

#include <papyro/libraryview.h>
#include <papyro/libraryview_p.h>
#include <papyro/librarymodel.h>
#include <utopia2/qt/spinner.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QItemEditorFactory>
#include <QLineEdit>
#include <QMetaMethod>
#include <QMimeData>
#include <QModelIndex>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QStackedLayout>

#include <QDebug>

class QMimeData;

namespace Athenaeum
{

    LibraryStatusIconPrivate::LibraryStatusIconPrivate(LibraryStatusIcon * label)
        : QObject(label), label(label)
    {}

    LibraryStatusIconPrivate::~LibraryStatusIconPrivate()
    {}

    bool LibraryStatusIconPrivate::eventFilter(QObject * object, QEvent * event)
    {
        qDebug() << "filtering" << object << event;
        if (object == editor) {
            switch (event->type()) {
            case QEvent::FocusIn:
                layout->setCurrentIndex(1);
                editor->setFocus();
                break;
            case QEvent::FocusOut:
                layout->setCurrentIndex(0);
                break;
            default:
                break;
            }
        }
        return QObject::eventFilter(object, event);
    }

    void LibraryStatusIconPrivate::onStateChanged(Athenaeum::AbstractBibliography::State state)
    {
        spinner->setVisible(state == AbstractBibliography::BusyState);
        if (state == AbstractBibliography::BusyState) {
            spinner->start();
        } else {
            spinner->stop();
        }
    }




    LibraryStatusIcon::LibraryStatusIcon(QAbstractItemModel * model, const QModelIndex & parentIndex, int row)
        : QWidget(), d(new LibraryStatusIconPrivate(this))
    {
        d->layout = new QStackedLayout(this);
        d->layout->setSpacing(0);
        d->layout->setContentsMargins(0, 0, 0, 0);

        // Widget for spinner (default)
        QWidget * spinnerWidget = new QWidget;
        QHBoxLayout * hLayout = new QHBoxLayout(spinnerWidget);
        hLayout->setSpacing(0);
        hLayout->setContentsMargins(0, 0, 0, 0);
        d->spinner = new Utopia::Spinner;
        hLayout->addWidget(d->spinner, 1, Qt::AlignRight | Qt::AlignVCenter);
        d->spinner->setFixedSize(20, 14);
        d->spinner->hide();
        if (QAbstractItemModel * qaim = model->index(row, 0, parentIndex).data(LibraryModel::ModelRole).value< QAbstractItemModel * >()) {
            connect(qaim, SIGNAL(stateChanged(Athenaeum::AbstractBibliography::State)),
                    d, SLOT(onStateChanged(Athenaeum::AbstractBibliography::State)));
            // Initial spinner state
            if (AbstractBibliography * collection = qobject_cast< AbstractBibliography * >(qaim)) {
                d->onStateChanged(collection->state());
            }
        }
        d->layout->addWidget(spinnerWidget);

        // Text edit for editing
        //d->editor = qobject_cast< QLineEdit * >(QItemEditorFactory::defaultFactory()->createEditor(QVariant::String, this));
        //setFocusProxy(d->editor);
        //connect(d->editor, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged(const QString &)));
        //d->layout->addWidget(d->editor);
        //d->editor->installEventFilter(d);
        //setFocusPolicy(Qt::StrongFocus);
    }

    LibraryStatusIcon::~LibraryStatusIcon()
    {}

    void LibraryStatusIcon::setText(const QString & text)
    {
        d->editor->setText(text);
    }

    QString LibraryStatusIcon::text() const
    {
        return d->editor->text();
    }





    LibraryViewPrivate::LibraryViewPrivate(LibraryView * view)
        : QObject(view), view(view)
    {
        drop.row = -1;
        drop.dropping = false;
        drop.color = Qt::white;
        view->viewport()->installEventFilter(this);
        view->installEventFilter(this);
    }

    static bool canDecode(QAbstractItemModel * model, QDragEnterEvent * e)
    {
        QStringList modelTypes = model->mimeTypes();
        const QMimeData * mime = e->mimeData();
        for (int i = 0; i < modelTypes.count(); ++i) {
            if (mime->hasFormat(modelTypes.at(i))) {
                return true;
            }
        }
        return false;
    }

    bool LibraryViewPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        // Signature of method required for drag/drop functionality
        static const QByteArray accepts = QMetaObject::normalizedSignature(
            "acceptsDrop(const QModelIndex &, bool, const QMimeData *)"
        );
        static const int padding = 2;

        // Only filter the view's events
        if (view && view->model()) {
            if (obj == view->viewport()) {
                const QMetaObject * meta = view->model()->metaObject();
                int index = meta->indexOfMethod(accepts);
                if (index >= 0) {
                    QMetaMethod method = meta->method(index);
                    switch (event->type()) {
                    case QEvent::MouseMove:
                        if (drop.dropping) {
                            drop.dropping = false;
                            view->viewport()->update();
                            return true;
                        }
                        break;
                    case QEvent::MouseButtonPress: {
                        QMouseEvent * e = static_cast< QMouseEvent * >(event);
                        drop.pressed = e->pos();
                        break;
                    }
                    case QEvent::DragEnter: {
                        drop.index = QModelIndex();
                        drop.row = -1;
                        QDragEnterEvent * e = static_cast< QDragEnterEvent * >(event);

                        // Skip if not internal while expecting to be
                        if (view->dragDropMode() == QAbstractItemView::InternalMove
                            && (e->source() != view->viewport() || !(e->possibleActions() & Qt::MoveAction))) {
                            return true;
                        }

                        if ((drop.dropping = canDecode(view->model(), e))) {
                            e->accept();
                        } else {
                            e->ignore();
                        }

                        return true;
                    }
                    case QEvent::DragLeave: {
                        drop.index = QModelIndex();
                        drop.row = -1;
                        drop.dropping = false;
                        view->viewport()->update();
                        //qDebug() << "=== LEAVE" << drop.index << drop.row << drop.dropping;
                        return true;
                    }
                    case QEvent::DragMove: {
                        QDragMoveEvent * e = static_cast< QDragMoveEvent * >(event);
                        QModelIndex index(view->indexAt(e->pos()));
                        if (index.isValid()) {
                            // Would the item under the mouse accept the drop?
                            bool acceptsHere = true;
                            method.invoke(view->model(),
                                          Qt::DirectConnection,
                                          Q_RETURN_ARG(bool, acceptsHere),
                                          Q_ARG(QModelIndex, index),
                                          Q_ARG(bool, false),
                                          Q_ARG(const QMimeData *, e->mimeData()));

                            // If above / below, does the parent item accept the drop?
                            QRect rect(view->visualRect(index));
                            bool above(e->pos().y() <= rect.top() + (acceptsHere ? padding : rect.height() / 2));
                            bool below(e->pos().y() >= rect.bottom() - (acceptsHere ? padding : rect.height() / 2));
                            bool acceptsParent = false;
                            if (above || below) {
                                method.invoke(view->model(),
                                              Qt::DirectConnection,
                                              Q_RETURN_ARG(bool, acceptsParent),
                                              Q_ARG(QModelIndex, index.parent()),
                                              Q_ARG(bool, true),
                                              Q_ARG(const QMimeData *, e->mimeData()));
                            }

                            if (acceptsParent) {
                                drop.index = index.parent();
                                drop.row = index.row() + (below ? 1 : 0);
                            } else if (acceptsHere) {
                                drop.index = index;
                                drop.row = -1;
                            } else {
                                drop.index = QModelIndex();
                                drop.row = -1;
                            }
                        } else {
                            drop.row = -1;
                        }
                        if (drop.index.isValid() || drop.row > -1) {
                            QModelIndex dragged(view->indexAt(drop.pressed));
                            if (e->source() == view->viewport()) {
                                if ((drop.row == -1 && drop.index == dragged) ||
                                    (drop.index == dragged.parent() && (drop.row == dragged.row() || drop.row == dragged.row() + 1))) { // drop on self
                                    e->ignore();
                                    drop.index = QModelIndex();
                                    drop.row = -1;
                                    view->viewport()->update();
                                    return true;
                                }
                            }
                            e->accept();
                        } else {
                            e->ignore();
                        }
                        view->viewport()->update();
                        return true;
                    }
                    case QEvent::Drop: {
                        QDropEvent * e = static_cast< QDropEvent * >(event);

                        // Skip if not internal while expecting to be
                        if (view->dragDropMode() == QAbstractItemView::InternalMove
                            && (e->source() != view->viewport() || !(e->possibleActions() & Qt::MoveAction))) {
                            drop.index = QModelIndex();
                            drop.row = -1;
                            drop.dropping = false;
                            return true;
                        }

                        if (drop.index.isValid() || drop.row > -1) {
                            bool success = false;
                            if (drop.index.isValid() && drop.row == -1) {
                                success = view->model()->dropMimeData(e->mimeData(),
                                    view->dragDropMode() == QAbstractItemView::InternalMove ? Qt::MoveAction : e->dropAction(),
                                    drop.index.row(), 0, drop.index.parent());
                            } else {
                                success = view->model()->dropMimeData(e->mimeData(),
                                    view->dragDropMode() == QAbstractItemView::InternalMove ? Qt::MoveAction : e->dropAction(),
                                    drop.row, 0, drop.index);
                            }
                            if (success) {
                                e->acceptProposedAction();
                            } else {
                                e->ignore();
                            }
                            drop.index = QModelIndex();
                            drop.row = -1;
                            drop.dropping = false;
                        }
                        view->viewport()->update();
                        return true;
                    }
                    default:
                        break;
                    }
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }

    void LibraryViewPrivate::setIndexWidget(const QModelIndex & parent, int row)
    {
        if (parent.isValid()) {
            int rowCount(view->model()->rowCount(parent));
            if (row >= 0 && row < rowCount) {
                view->setIndexWidget(view->model()->index(row, 0, parent), new LibraryStatusIcon(view->model(), parent, row));
            }
        } else {
            QModelIndex newParent(view->model()->index(row, 0, parent));
            for (int i = 0; i < view->model()->rowCount(newParent); ++i) {
                setIndexWidget(newParent, i);
            }
        }
    }




    LibraryView::LibraryView(QWidget * parent)
        : QTreeView(parent), d(new LibraryViewPrivate(this))
    {}

    LibraryView::~LibraryView()
    {}

    QColor LibraryView::dropColor() const
    {
        return d->drop.color;
    }

    void LibraryView::paintEvent(QPaintEvent * e)
    {
        QTreeView::paintEvent(e);

        // Call viewport's paint event
        if (d->drop.dropping && viewport() && (d->drop.index.isValid() || d->drop.row >= 0)) {
            // Paint on top
            QPainter painter(viewport());
            painter.setRenderHint(QPainter::Antialiasing, true);
            QColor color(dropColor());
            painter.setPen(QPen(color, 2));
            color.setAlpha(80);
            painter.setBrush(color);
            // If the index is set, highlight the row
            if (d->drop.row == -1 && d->drop.index.isValid()) {
                QRect rect(visualRect(d->drop.index).adjusted(1, 1, -1, -1));
                painter.drawRoundedRect(rect, 4, 4);
            }
            // If the index has a highlighted child position
            if (d->drop.row > -1) {
                int childCount = model()->rowCount(d->drop.index);
                QModelIndex childIndex(model()->index(qMin(d->drop.row, childCount-1), 0, d->drop.index));
                QRect rect(visualRect(childIndex));
                if (d->drop.row == childCount) {
                    rect.moveTop(rect.top() + 1);
                    painter.drawLine(rect.bottomLeft(), rect.bottomRight());
                } else {
                    if (!d->drop.index.isValid() && d->drop.row == 0) {
                        rect.moveTop(rect.top() + 1);
                    }
                    painter.drawLine(rect.topLeft(), rect.topRight());
                }
            }
        }
    }

    void LibraryView::rowsInserted(const QModelIndex & parent, int start, int end)
    {
        QTreeView::rowsInserted(parent, start, end);
        for (int i = start; i <= end; ++i) {
            //d->setIndexWidget(parent, i);
        }
        expandAll();
    }

    void LibraryView::setDropColor(const QColor & color)
    {
        d->drop.color = color;
    }

} // namespace Athenaeum
