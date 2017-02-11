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

#include "commenting.h"

#include <papyro/commentdata.h>
#include <papyro/documentview.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>
#include <utopia2/global.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <QtDebug>




namespace
{
    bool SortDateDescending(const Papyro::CommentData * lhs, const Papyro::CommentData * rhs)
    {
        return lhs->date() < rhs->date();
    }
}




/////////////////////////////////////////////////////////////////////////////////////////
/// CommentProcessorController

CommentProcessorController::CommentProcessorController(Spine::DocumentHandle document, Spine::AnnotationSet annotations, Utopia::Conversation * conversation)
    : QObject(conversation), document(document), selection(document->textSelection()), annotations(annotations), conversation(conversation)
{
    conversation->setWindowTitle("Discussion");

    // If there's a tree of comments already, order and add
    std::list< Papyro::CommentData * > ordered;
    foreach (Spine::AnnotationHandle annotation, annotations) {
        ordered.push_back(new Papyro::CommentData(annotation));
    }
    ordered.sort(SortDateDescending);
    foreach (Papyro::CommentData * child, ordered) {
        addComment(child);
    }

    connect(conversation, SIGNAL(newComment(const QString &, bool, const QString &)),
            this, SLOT(onSubmit(const QString &, bool, const QString &)));
    connect(conversation, SIGNAL(deleteMyComment(const QString &)),
            this, SLOT(onDelete(const QString &)));
    connect(conversation, SIGNAL(publishMyComment(const QString &, bool)),
            this, SLOT(onPublish(const QString &, bool)));

    if (annotations.empty()) {
        conversation->allowAddCommentField();
    }
}

void CommentProcessorController::addComment(Papyro::CommentData * comment)
{
    // Add comment
    conversation->addComment(comment);

    // Add children
    Spine::AnnotationSet children(document->annotationsByParentId(Papyro::unicodeFromQString(comment->id())));
    std::list< Papyro::CommentData * > ordered;
    foreach (Spine::AnnotationHandle annotation, children) {
        ordered.push_back(new Papyro::CommentData(annotation));
    }
    ordered.sort(SortDateDescending);
    foreach (Papyro::CommentData * child, ordered) {
        addComment(child);
    }
}

void CommentProcessorController::onSubmit(const QString & text, bool isPublic, const QString & parent)
{
    //std::cout << "Submit comment " << text.toStdString() << " (except not yet)" << std::endl;

    // Make annotation and send to server
    Spine::AnnotationHandle annotation(new Spine::Annotation);
    annotation->setProperty("concept", "UserComment");
    annotation->setProperty("property:comment", Papyro::unicodeFromQString(text));
    annotation->setPublic(isPublic);
    if (parent.isEmpty()) {
        bool added = false;
        foreach (Spine::TextExtentHandle extent, selection) {
            annotation->addExtent(extent);
            added = true;
        }
        if (!added && !annotations.empty()) {
            foreach (Spine::TextExtentHandle extent, (*annotations.begin())->extents()) {
                annotation->addExtent(extent);
            }
        }
    } else {
        annotation->setProperty("parent", Papyro::unicodeFromQString(parent));
    }

    document->addAnnotation(annotation, "PersistQueue");

    if (!annotation->getFirstProperty("id").empty()) {
        conversation->newCommentSuccess();
        conversation->addComment(new Papyro::CommentData(annotation));
    } else {
        document->removeAnnotation(annotation, "PersistQueue");
        conversation->newCommentFail();
    }
}

void CommentProcessorController::onDelete(const QString & uri)
{
    // std::cout << "Delete comment " << uri.toStdString() << std::endl;

    // Only destroy child-less comments
    Spine::AnnotationSet children = document->annotationsByParentId(Papyro::unicodeFromQString(uri));
    if (children.empty()) {
        Spine::AnnotationSet annotations = document->annotationsById(Papyro::unicodeFromQString(uri));
        if (annotations.size() > 0) {
            foreach (Spine::AnnotationHandle annotation, annotations) {
                document->addAnnotation(annotation, document->deletedItemsScratchId());
            }

            Spine::AnnotationSet failed = document->annotations(document->deletedItemsScratchId());
            foreach (Spine::AnnotationHandle annotation, annotations) {
                if (failed.find(annotation) == failed.end()) {
                    document->removeAnnotation(annotation);
                }
            }
        }

        if (document->annotations(document->deletedItemsScratchId()).empty()) {
            conversation->deleteCommentSuccess();
            conversation->removeComment(uri);
        } else {
            conversation->deleteCommentFail();
        }
    } else {
        conversation->deleteCommentFail();
    }
}

void CommentProcessorController::onPublish(const QString & uri, bool publish)
{
    //std::cout << "Publish comment " << uri.toStdString() << " (except not yet)" << std::endl;

    Spine::AnnotationSet anns = document->annotationsById(Papyro::unicodeFromQString(uri));
    // If there is more than one annotation with the same URI, we've got a problem!? [!]
    Spine::AnnotationHandle annotation = *(anns.begin());

    annotation->setPublic(publish);
    std::string oldRevision = annotation->getFirstProperty("revision");
    document->addAnnotation(annotation, "PersistQueue");

    // tab->publishChanges(); FIXME

    std::string newRevision = annotation->getFirstProperty("revision");
    if (oldRevision != newRevision) {
        conversation->publishCommentSuccess();
        conversation->refreshComment(uri);
    } else {
        document->removeAnnotation(annotation, "PersistQueue");
        conversation->publishCommentFail();
    }
}




/////////////////////////////////////////////////////////////////////////////////////////
/// CommentProcessor

void CommentProcessor::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    Utopia::Conversation * conversation = new Utopia::Conversation;
    /* CommentProcessorController * controller = */ new CommentProcessorController(document, annotations, conversation);

#if defined(Q_OS_LINUX)
    if (Papyro::PapyroWindow * window = Papyro::PapyroWindow::currentWindow()) {
        if (Papyro::PapyroTab * tab = window->currentTab()) {
            QSize windowSize(tab->window()->size());
            QSize convoSize(conversation->size());
            conversation->move(tab->window()->pos() + QPoint(windowSize.width(), windowSize.height()) / 2 - QPoint(convoSize.width(), convoSize.height()) / 2);
        }
    }
#endif
    conversation->show();
    conversation->raise();
}

QList< Spine::AnnotationSet > CommentProcessor::canActivate(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    Spine::AnnotationSet valid;
    foreach (Spine::AnnotationHandle annotation, annotations) {
        if (canActivate(document, annotation)) {
            valid.insert(annotation);
        }
    }
    if (valid.empty()) {
        return QList< Spine::AnnotationSet >();
    } else {
        return QList< Spine::AnnotationSet >() << valid;
    }
}

bool CommentProcessor::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return annotation->getFirstProperty("concept") == "UserComment";
}

int CommentProcessor::category() const
{
    return -10;
}

QIcon CommentProcessor::icon() const
{
    return generateFromMonoPixmap(QPixmap(":/processors/commenting/icon.png"));
}

QIcon CommentProcessor::icon(Spine::DocumentHandle document, Spine::AnnotationHandle /*annotation*/) const
{
    return icon();
}

void CommentProcessor::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
{
    activate(document, Spine::AnnotationSet(), globalPos);
}

QString CommentProcessor::title(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    return title();
}

QString CommentProcessor::title() const
{
    return "Comment On This...";
}

int CommentProcessor::weight() const
{
    return 20;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// CommentProcessorFactory

QList< boost::shared_ptr< Papyro::SelectionProcessor > > CommentProcessorFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
{
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
    QVariantMap defaults(Utopia::defaults());
    QString method = defaults.value("service_method").toString();
    if (method != "prevent" && hasTextSelection(document, cursor)) {
        list << boost::shared_ptr< Papyro::SelectionProcessor >(new CommentProcessor);
    }
    return list;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// CommentRenderer

CommentRenderer::CommentRenderer()
    : Papyro::SvgMarginaliaOverlayRenderer(":/processors/commenting/margin-icon.svg")
{}

QString CommentRenderer::id()
{
    return "comment";
}

int CommentRenderer::weight()
{
    return 10;
}
