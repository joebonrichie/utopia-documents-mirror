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

#include <papyro/articleview.h>
#include <papyro/articleview_p.h>
#include <papyro/articledelegate.h>
#include <papyro/resolverrunnable.h>
#include <papyro/bibliographicmimedata_p.h>
#include <papyro/exporter.h>
#include <papyro/librarymodel.h>

#include <QCheckBox>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaMethod>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QPushButton>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QVector>

#include <QDebug>

class QMimeData;

namespace Athenaeum
{

    ArticleViewPrivate::ArticleViewPrivate(ArticleView * view)
        : QObject(view), view(view), dropping(false)
    {
        view->viewport()->installEventFilter(this);

        foreach (Exporter * exporter, Utopia::instantiateAllExtensions< Exporter >()) {
            exporters[exporter->name()] = exporter;
        }

        updateTimer.setInterval(10);
        connect(&updateTimer, SIGNAL(timeout()), view, SLOT(update()));

        libraryModel = Athenaeum::LibraryModel::instance();
    }

    ArticleViewPrivate::~ArticleViewPrivate()
    {
        // cancel any runnables
        cancelRunnables();

        foreach (Exporter * exporter, exporters.values()) {
            delete exporter;
        }
    }

    static QAbstractItemModel * origin(QAbstractItemModel * model)
    {
        if (QSortFilterProxyModel * proxy = qobject_cast< QSortFilterProxyModel * >(model)) {
            return origin(proxy->sourceModel());
        } else {
            return model;
        }
    }

    bool ArticleViewPrivate::eventFilter(QObject * obj, QEvent * event)
    {
        //qDebug() << "eventFilter" << obj << event;

        QAbstractItemModel * model = view ? origin(view->model()) : 0;

        // Only filter the view's events
        if (model && obj == view->viewport()) {
            switch (event->type()) {
            case QEvent::DragMove:
            case QEvent::DragEnter: {
                QDropEvent * e = static_cast< QDropEvent * >(event);
                /*
                if ((dropping = (dndmodel && dndmodel->acceptsDrop(e->mimeData())))) {
                    view->viewport()->update();
                    e->accept();
                } else {
                */
                    e->ignore();
                /*
                }
                */
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

    void ArticleViewPrivate::exportArticlesAsCitations(const QItemSelection & selection)
    {
        // Get a filename for the citation export as well as which exporter to use
        QString s("s"), ies("ies");
        if (selection.size() == 1 && selection.at(0).height() == 1 && selection.at(0).width() == 1) {
            s = "";
            ies = "y";
        }

        // Compile information for file dialog
        QMap< QString, Exporter * > formats;
        foreach (Exporter * exporter, exporters.values()) {
            QString format = QString("%1 (*.%2)").arg(exporter->name(), exporter->extensions().join(" *."));
            formats[format] = exporter;
        }
        QString filter = QStringList(formats.keys()).join(";;");

        // Request an export filename from the user
        QString selectedFilter;
        QString filename = QFileDialog::getSaveFileName(view->window(), "Export citation"+s, QString(), filter, &selectedFilter);
        Exporter * exporter = formats.value(selectedFilter, 0);
        if (exporter && !filename.isEmpty()) {
            exporter->doExport(selection.indexes(), filename);
        }
    }

    void ArticleViewPrivate::exportAllArticlesAsCitations()
    {
        QAbstractItemModel * model = view->model();
        QItemSelection selection(model->index(0, 0),
                                 model->index(model->rowCount()-1, 0));
        exportArticlesAsCitations(selection);
    }

    void ArticleViewPrivate::exportSelectedArticlesAsCitations()
    {
        exportArticlesAsCitations(view->selectionModel()->selection());
    }

    void ArticleViewPrivate::onArticleActivated(const QModelIndex & index)
    {
        emit articleActivated(index, false);
    }

    void ArticleViewPrivate::cancelRunnables()
    {
        emit cancellationRequested();
    }

    void ArticleViewPrivate::onResolverRunnableCompleted(QVariantMap metadata)
    {
        bool raise = metadata.value("__raise").toBool();
        QModelIndex index = metadata.value("__index").value< QModelIndex >();

        view->model()->setData(index, AbstractBibliography::IdleState, Citation::StateRole);

        emit articleActivated(index, raise);
    }

    QModelIndexList ArticleViewPrivate::selectedIndexes() const
    {
        QModelIndexList indexes;
        foreach (QModelIndex index, view->selectionModel()->selectedIndexes()) {
            if (index.column() == 0) {
                indexes << index;
            }
        }
        return indexes;
    }

    void ArticleViewPrivate::openSelectedArticles()
    {
        emit articlesActivated(selectedIndexes(), false);
    }

    void ArticleViewPrivate::openSelectedArticlesInNewWindow()
    {
        emit articlesActivated(selectedIndexes(), true);
    }

    void ArticleViewPrivate::removeSelectedArticlesFromLibrary()
    {
        // Get all the articles doomed for deletion
        QModelIndexList doomed = selectedIndexes();
        if (doomed.isEmpty()) {
            QMessageBox::information(view, "Oops...",
                                     "There are no articles selected, so nothing"
                                     " has been removed from the library.");
            return;
        }
        const QString art_s(doomed.size() > 1 ? "s" : "");
        const QString art_t(doomed.size() > 1 ? "these" : "this");
        qSort(doomed.begin(), doomed.end(), qGreater< QModelIndex >());

        // Do any have related PDF files?
        size_t pdfsFound = 0;
        foreach (const QModelIndex & index, doomed) {
            QUrl path(index.data(Citation::ObjectFileRole).toUrl());
            if (path.isLocalFile()) {
                QFileInfo info(path.toLocalFile());
                if (info.exists()) {
                    ++pdfsFound;
                }
            }
        }

        const QString pdf_s(pdfsFound > 1 ? "s" : "");
        const QString pdf_t(pdfsFound > 1 ? "these" : "this");

        QDialog box(view);
        box.setWindowTitle("Are you sure?");
        QVBoxLayout * layout = new QVBoxLayout(&box);
        layout->setContentsMargins(15, 15, 15, 10);
        layout->addWidget(new QLabel("Are you sure you wish to remove the selected article"+art_s+" from your library?"));
        QCheckBox * unlinkCheck = 0;
        // Ask the user whether files should be deleted as well
        if (pdfsFound > 0) {
            unlinkCheck = new QCheckBox("Delete from disk too.");
            layout->addWidget(unlinkCheck);
        }
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->addStretch(1);
        QPushButton * cancelButton = new QPushButton("Cancel");
        connect(cancelButton, SIGNAL(clicked()), &box, SLOT(reject()));
        buttonLayout->addWidget(cancelButton);
        QPushButton * yesButton = new QPushButton("Yes");
        connect(yesButton, SIGNAL(clicked()), &box, SLOT(accept()));
        buttonLayout->addWidget(yesButton);
        layout->addLayout(buttonLayout);
        box.exec();

        if (box.result() == QDialog::Accepted) {
            bool unlink = unlinkCheck && unlinkCheck->isChecked();
            foreach (const QModelIndex & index, doomed) {
                CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                citation->setField(Citation::DateImportedRole, QVariant());
                if (unlink) {
                    QUrl path(index.data(Citation::ObjectFileRole).toUrl());
                    if (path.isLocalFile()) {
                        QFile file(path.toLocalFile());
                        if (file.exists()) {
                            // Delete from filesystem
                            file.remove();
                        }
                    }
                }
                Athenaeum::Bibliography * master = libraryModel->master();
                citation->setField(Athenaeum::Citation::DateImportedRole, QVariant());
                master->removeItem(citation);
            }
        }
    }

    void ArticleViewPrivate::saveSelectedArticlesToLibrary()
    {
        QVector< CitationHandle > toBeAdded;
        foreach (QModelIndex index, selectedIndexes()) {
            if (CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >()) {
                if (!citation->isKnown()) {
                    toBeAdded << citation;
                }
            }
        }
        if (!toBeAdded.isEmpty()) {
            // Add to library
            Bibliography * master = libraryModel->master();
            foreach (CitationHandle citation, toBeAdded) {
                citation->setField(Citation::DateImportedRole, QDateTime::currentDateTime());
            }
            master->prependItems(toBeAdded);
        }
    }




    ArticleView::ArticleView(QWidget * parent)
        : QListView(parent), d(new ArticleViewPrivate(this))
    {
#ifndef Q_OS_WIN
        QFont f(font());
        f.setPointSizeF(f.pointSizeF() * 0.85);
        setFont(f);
#endif
        setMouseTracking(true);
        setDragEnabled(true);
        setAcceptDrops(true);
        setDropIndicatorShown(false);
        setDefaultDropAction(Qt::MoveAction);
        ArticleDelegate * delegate = new ArticleDelegate(this);
        connect(delegate, SIGNAL(updateRequested()), this, SLOT(update()));
        setItemDelegate(delegate);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        horizontalScrollBar()->disconnect(); // no horizontal scrolling
        setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        setAlternatingRowColors(false);
        setFrameStyle(QFrame::NoFrame);
        setAttribute(Qt::WA_MacShowFocusRect, false);
        setDragDropMode(QAbstractItemView::DragDrop);

        connect(this, SIGNAL(activated(const QModelIndex &)),
                d, SLOT(onArticleActivated(const QModelIndex &)));
        connect(d, SIGNAL(articleActivated(const QModelIndex &, bool)),
                this, SIGNAL(articleActivated(const QModelIndex &, bool)));
        connect(d, SIGNAL(articlesActivated(const QModelIndexList &, bool)),
                this, SIGNAL(articlesActivated(const QModelIndexList &, bool)));
    }

    ArticleView::~ArticleView()
    {}

    void ArticleView::contextMenuEvent(QContextMenuEvent * event)
    {
        // Get the current selection
        int count_selected = selectionModel()->selectedIndexes().size();
        if (count_selected > 0) {
            int count_all = model()->rowCount();
            int count_known = 0;
            int count_starred = 0;
            foreach (QModelIndex index, selectionModel()->selectedIndexes()) {
                CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                if (citation->isKnown()) { ++count_known; }
                if (citation->isStarred()) { ++count_starred; }
            }
            int count_unknown = count_selected-count_known;
            int count_unstarred = count_selected-count_starred;

            // For use in plurals
            QString s_selected(count_selected > 1 ? "s" : "");
            QString s_all(count_all > 1 ? "s" : "");
            QString s_known(count_known > 1 ? "s" : "");
            QString s_unknown(count_unknown > 1 ? "s" : "");
            QString s_starred(count_starred > 1 ? "s" : "");
            QString s_unstarred(count_unstarred > 1 ? "s" : "");

            // New menu
            QMenu menu;

            menu.addAction("Open", d, SLOT(openSelectedArticles()));
            connect(menu.addAction("Open in New Window"), SIGNAL(triggered()),
                    d, SLOT(openSelectedArticlesInNewWindow()), Qt::QueuedConnection);

            menu.addSeparator();
            if (count_unknown > 0) {
                menu.addAction("Save to Library", d, SLOT(saveSelectedArticlesToLibrary()));
            }
            if (count_known > 0) {
                menu.addAction("Remove from Library", d, SLOT(removeSelectedArticlesFromLibrary()));
            }

            if (!d->exporters.isEmpty()) {
                menu.addSeparator();
                menu.addAction("Export as "+QString(count_selected==1?"a ":"")+"Citation"+s_selected+"...", d, SLOT(exportSelectedArticlesAsCitations()));
                if (count_selected < count_all) {
                    menu.addAction("Export "+QString(count_all>1?"All ":"")+"as "+QString(count_all==1?"a ":"")+"Citation"+s_all+"...", d, SLOT(exportAllArticlesAsCitations()));
                }
            }

            menu.exec(event->globalPos());
        }
    }

    void ArticleView::dataChanged(const QModelIndex & topLeft,
                                  const QModelIndex & bottomRight,
                                  const QVector< int > & roles)
    {
        QListView::dataChanged(topLeft, bottomRight, roles);

        if (roles.isEmpty() || roles.contains(Citation::FlagsRole)) {
            for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                for (int column = topLeft.column(); column <= bottomRight.column(); ++column) {
                    QModelIndex index(topLeft.sibling(row, column));
                    CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                    if (citation) {
                        bool isStarred = citation->isStarred();
                        bool isKnown = citation->isKnown();

                        if (isStarred && !isKnown) {
                            // Add to library
                            Bibliography * master = d->libraryModel->master();
                            master->prependItem(citation);
                        }
                    }
                }
            }
        }
        if (roles.isEmpty() || roles.contains(Citation::StateRole)) {
            bool isBusy = false;
            for (int row = 0; row < model()->rowCount(); ++row) {
                QModelIndex index(model()->index(row, 0));
                CitationHandle citation = index.data(Citation::ItemRole).value< CitationHandle >();
                if (citation && citation->isBusy()) {
                    isBusy = true;
                    break;
                }
            }
            if (isBusy) {
                d->updateTimer.start();
            } else {
                d->updateTimer.stop();
            }
        }
    }

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

    void ArticleView::reset()
    {
        QListView::reset();
        dataChanged(model()->index(0, 0), model()->index(model()->rowCount() - 1, 0));
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
