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

#include <utopia2/auth/qt/conversation.h>
#include <utopia2/auth/qt/comment.h>
#include <utopia2/auth/qt/commenteditor.h>
#include <utopia2/qt/messagelabel.h>
#include <utopia2/qt/tearout.h>

#include <QCheckBox>
#include <QPalette>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QPushButton>
#include <QDebug>

typedef QMap< QString, QString > QStringMap;

namespace Utopia
{
    Conversation::Conversation(QWidget * parent) : QWidget(parent)
    {
        // General setup
        //showCloseButton(true);
        this->setMinimumWidth(460);
        this->resize(460, 400);
        this->setContentsMargins(0, 0, 0, 0);

        // Message label
        this->_messageLabel = new MessageLabel();

        // Status bar
        _statusBar = new QStatusBar();
        _statusBar->setFixedHeight(32);
        _addButton = new QPushButton();
        _addButton->setText("Add New Comment");
        connect(_addButton, SIGNAL(clicked()), this, SLOT(showCommentField()));
        _statusBar->addWidget(_addButton, 0);

        // Scroll area and its view widget
        _centralScrollArea = new QScrollArea();
        _centralScrollArea->setContentsMargins(0, 0, 0, 0);
        _centralScrollArea->setWidgetResizable(true);
        _centralScrollArea->setFrameStyle(QFrame::NoFrame);

        _scrollWidget = new QWidget();
        _scrollWidget->setContentsMargins(0, 0, 0, 0);
        _scrollWidget->setObjectName("conversationArea");
        _centralScrollArea->setWidget(_scrollWidget);
        //_centralScrollArea->setMinimumHeight(0);
        //_centralScrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

        // Layouts
        _layout = new QVBoxLayout();
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);
        _spacer = new QSpacerItem(10, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        _layout->addSpacerItem(_spacer);
        _scrollWidget->setLayout(_layout);

        _outerLayout = new QVBoxLayout(this);
        _outerLayout->setContentsMargins(0, 0, 0, 0);
        _outerLayout->setSpacing(0);
        _outerLayout->addWidget(_centralScrollArea);
        _outerLayout->addWidget(_statusBar);

        /*_centralWidget = new QWidget;
        _centralWidget->setContentsMargins(0, 0, 0, 0);
        _centralWidget->setLayout(_outerLayout);
        setCentralWidget(_centralWidget);*/

        // Scroll area
        _centralScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // New comment editor
        _editorNewComment = new CommentEditor;
        _editorNewComment->setMinimumHeight(160);
        _editorNewComment->setContentsMargins(0, 0, 0, 0);
        _editorNewComment->setStandalone();
        _editorNewComment->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
        connect(_editorNewComment, SIGNAL(submitted()), this, SLOT(submitNewCommentClicked()));
        connect(_editorNewComment, SIGNAL(cancelled()), this, SLOT(hideCommentField()));
        connect(_editorNewComment, SIGNAL(saved()), this, SLOT(saveNewCommentClicked()));
        _outerLayout->insertWidget(1, _editorNewComment);

        // Reply editor
        _editorReply = new CommentEditor;
        _editorReply->setContentsMargins(0, 0, 0, 0);
        _editorReply->setFixedHeight(160);
        connect(_editorReply, SIGNAL(submitted()), this, SLOT(submitReplyClicked()));
        connect(_editorReply, SIGNAL(cancelled()), this, SLOT(hideReplyField()));
        connect(_editorReply, SIGNAL(saved()), this, SLOT(saveReplyClicked()));
        _activeParent = 0;

        _spinner = new Spinner;
        _spinner->setFixedSize(32, 24);
        _spinner->setColor(QColor(40, 40, 40));
        _statusBar->addWidget(_spinner, 0);
        _statusBar->addWidget(_messageLabel, 1);

        QVariantMap defaults(Utopia::defaults());
        QString method = defaults.value("service_method").toString();
        _addButtonAllowed = (method != "prevent");
        _addButton->setVisible(_addButtonAllowed);
        _editorNewComment->hide();
        _centralScrollArea->viewport()->update();
    }

    void Conversation::addComment(AbstractCommentData * commentData)
    {
        //qDebug() << "addComment() with data " << commentData->text();
        // If the comment is null, do nothing
        if (commentData == 0)
        {
            return;
        }

        // If the comment already exists, update it
        if (_idToCommentMap.contains(commentData->id()))
        {
            // update comment

            return;
        }

        // Get the parent, if it has one
        Comment * parent = _idToCommentMap.value(commentData->parent(), 0);

        // Control editing of a user's own comments (TODO)
        bool authorsOwn = false;
        // Make a new comment object
        Comment * c = new Comment(parent, commentData, authorsOwn);

        // Put the new comment into the map, and the data in the data map
        _idToCommentMap[commentData->id()] = c;
        _commentToDataMap[c] = commentData;

        // Connect the reply button
        connect(c, SIGNAL(replyPressed()), this, SLOT(showReplyField()));
        // Connect the publish button
        connect(c, SIGNAL(publishPressed(QString)), this, SLOT(publishComment(QString)));
        // Connect the delete button
        connect(c, SIGNAL(deletePressed(QString)), this, SLOT(deleteComment(QString)));

        // Insert the comment into the view (TODO)
        insertComment(c);
    }

    void Conversation::allowAddCommentField()
    {
        QVariantMap defaults(Utopia::defaults());
        QString method = defaults.value("service_method").toString();
        _addButtonAllowed = (method != "prevent");
        showCommentField();
    }

    AbstractCommentData * Conversation::getRoot() const
    {
        if (_layout->count() == 0)
        {
            return 0;
        }
        else
        {
            Comment * c = static_cast< Comment * >(_layout->itemAt(0)->widget());
            return this->_commentToDataMap.value(c, 0);
        }
    }

    // What about children?? We assume it only gets called on child-less comments [!]
    void Conversation::removeComment(QString uri)
    {
        Comment * c = this->_idToCommentMap[uri];
        this->_idToCommentMap.remove(uri);
        this->_commentToDataMap.remove(c);
        delete c;
    }

    void Conversation::refreshComment(QString uri)
    {
        Comment * c = this->_idToCommentMap[uri];
        c->refresh(); // This ensures the comments stays in the same place
        // as opposed to, say, remove old comment and add an new updated version instead
    }

    void Conversation::hideCommentField()
    {
        _editorNewComment->hide();
        _centralScrollArea->show();

        if (_addButtonAllowed)
        {
            _addButton->show();
        }

        if (_idToCommentMap.isEmpty())
        {
            close();
        }
    }

    void Conversation::hideReplyField()
    {
        _layout->removeWidget(_editorReply);
        _editorReply->hide();
        update();
    }

    namespace
    {

        int skipToEnd(QVBoxLayout * layout, Comment * parent)
        {
            int index = layout->indexOf(parent);
            Comment * prev = 0;
            // Progress through layout
            while (QLayoutItem * item = layout->itemAt(++index))
            {
                Comment * next = static_cast< Comment * >(item->widget());
                if (next->parentComment() == parent)
                {
                    // Skip
                    prev = next;
                }
                else if (next->parentComment() && next->parentComment() == prev)
                {
                    // Gone down a level in the hierarchy
                    index = skipToEnd(layout, next) - 1;
                }
                else
                {
                    // Gone past the end, so bail
                    break;
                }
            }
            return index;
        }

    }

    int Conversation::insertComment(Comment * comment)
    {
        //qDebug() << "inserting comment" << comment;
        // Bail if the comment is null
        if (comment == 0)
        {
            return -1;
        }

        // Remove the spacer
        _layout->removeItem(_spacer);

        // Insert the comment
        if (comment->parentComment())
        {
            _layout->insertWidget(skipToEnd(_layout, comment->parentComment()), comment);
        }
        else
        {
            Tearout * tearout = new Tearout(_commentToDataMap.value(comment)->anchor());
            _layout->addWidget(tearout);
            _layout->addWidget(comment);
        }

        // Add the spacer
        _layout->addSpacerItem(_spacer);

        return 0;
    }

    void Conversation::newCommentFail()
    {
        _spinner->stop();

        // If this was a reply, leave the reply field
        if (!_editorReply->isEnabled())
        {
            _editorReply->setEnabled(true);
        }

        // If this was a new comment, leave the new comment field
        if (!_editorNewComment->isEnabled())
        {
            _editorNewComment->setEnabled(true);
        }

        update();
        _messageLabel->setInfoText("There was a problem adding your comment", false);
    }

    void Conversation::publishCommentFail()
    {
        _spinner->stop();
        update();
        _messageLabel->setInfoText("There was a problem publishing your comment", false);
    }

    void Conversation::deleteCommentFail()
    {
        _spinner->stop();
        update();
        _messageLabel->setInfoText("There was a problem deleting your comment", false);
    }

    void Conversation::newCommentSuccess()
    {
        _spinner->stop();

        if (!_editorReply->isEnabled())
        {
            _editorReply->setEnabled(true);
            _editorReply->clear();
            _layout->removeWidget(_editorReply);
            _editorReply->hide();
        }

        if (!_editorNewComment->isEnabled())
        {
            _editorNewComment->setEnabled(true);
            _editorNewComment->clear();
            _editorNewComment->hide();
            _centralScrollArea->show();
        }
        if (_addButtonAllowed)
        {
            _addButton->show();
        }

        update();
        _messageLabel->setInfoText("Your comment has been added", true);
    }

    void Conversation::deleteCommentSuccess()
    {
        _spinner->stop();
        update();
        _messageLabel->setInfoText("Your comment has been deleted", true);
    }

    void Conversation::publishCommentSuccess()
    {
        _spinner->stop();
        update();
        _messageLabel->setInfoText("Your comment has been published", true);
    }

    void Conversation::submitNewCommentClicked()
    {
        //qDebug() << "adding " << _editorNewComment->text() ;

        // Disable editor and wait for response
        _spinner->start();
        _editorNewComment->setEnabled(false);

        emit newComment(_editorNewComment->text(), true, QString());
    }

    void Conversation::saveNewCommentClicked()
    {
        //qDebug() << "adding " << _editorNewComment->text() ;

        // Disable editor and wait for response
        _spinner->start();
        _editorNewComment->setEnabled(false);

        emit newComment(_editorNewComment->text(), false, QString());
     }

    void Conversation::submitReplyClicked()
    {
        //qDebug() << "adding " << _editorReply->text() << ", public = " << _activeParent->isPublic();

        // Disable editor and wait for response
        _editorReply->setEnabled(false);
        _spinner->start();

        //setWindowModality(Qt::ApplicationModal);
        //setWindowFlags(Qt::Widget);
        emit newComment(_editorReply->text(), _activeParent->isPublic(), _activeParent->id());
    }

    void Conversation::saveReplyClicked() // Very similar to submitReplyClicked => factorize?
    {
        //qDebug() << "adding " << _editorReply->text() << ", public = " << false;

        // Disable editor and wait for response
        _editorReply->setEnabled(false);
        _spinner->start();

        //setWindowModality(Qt::ApplicationModal);
        //setWindowFlags(Qt::Widget);
        emit newComment(_editorReply->text(), false, _activeParent->id());
    }

    void Conversation::publishComment(QString uid)
    {
        _spinner->start();
        emit publishMyComment(uid, true);
    }

    void Conversation::deleteComment(QString uid)
    {
        _spinner->start();
        emit deleteMyComment(uid);
    }

    void Conversation::showReplyField()
    {
        if (!_spinner->active())
        {
            Comment * c = static_cast< Comment * >(sender());
            if (c != 0)
            {
                hideCommentField();
                _layout->removeWidget(_editorReply);
                // If this is the last comment in the view, use its parent instead
                Comment * parent = c;
                if ((_layout->count() - 2) == _layout->indexOf(c))
                {
                    parent = _idToCommentMap.value(c->commentData()->parent(), c);
                }
                _editorReply->setParentComment(parent);
                _layout->insertWidget(_layout->indexOf(c) + 1, _editorReply);
                _editorReply->clear();
                _activeParent = parent->commentData();
                _editorReply->show();
                _editorReply->setFocus(Qt::OtherFocusReason);
                _centralScrollArea->ensureVisible(_editorReply->x() + _editorReply->frameGeometry().width(),
                                                  _editorReply->y() + _editorReply->frameGeometry().height());
                _centralScrollArea->ensureVisible(_editorReply->x(), _editorReply->y());
                _centralScrollArea->update();
            }
        }
    }

    void Conversation::showCommentField()
    {
        if (!_spinner->active())
        {
            _addButton->hide();
            _editorNewComment->clear();
            _editorNewComment->show();
            if (_commentToDataMap.empty())
            {
                _centralScrollArea->hide();
            }

            _layout->removeWidget(_editorReply);
            _editorReply->hide();

            if (_idToCommentMap.isEmpty())
            {
                this->adjustSize();
            }
        }
    }

} // namespace Utopia
