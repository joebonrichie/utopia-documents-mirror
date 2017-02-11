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

#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <utopia2/auth/qt/config.h>
#include <utopia2/qt/abstractcommentdata.h>
#include <utopia2/qt/spinner.h>

#include <QMap>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QCheckBox;
class QLabel;
class QScrollArea;
class QSpacerItem;
class QStatusBar;
class QTextEdit;
class QTimer;
class QPushButton;

namespace Utopia
{
    class Comment;
    class CommentEditor;
    class MessageLabel;

    class LIBUTOPIA_AUTH_QT_API Conversation : public QWidget
    {
        Q_OBJECT

    public:
        Conversation(QWidget * parent = 0);

        void addComment(AbstractCommentData * comment);
        AbstractCommentData * getRoot() const;

        void removeComment(QString uri); // called by pageview
        void refreshComment(QString uri); // called by pageview

    protected:
        int insertComment(Comment * comment);

    protected slots:
        void showCommentField();
        void submitReplyClicked();
        void submitNewCommentClicked();
        void showReplyField();
        void hideReplyField();
        void hideCommentField();

        void saveReplyClicked();
        void saveNewCommentClicked();

        void publishComment(QString uid);
        void deleteComment(QString uid);

    public slots:
        void allowAddCommentField();
        void newCommentFail();
        void newCommentSuccess();

        void publishCommentFail();
        void publishCommentSuccess();
        void deleteCommentFail();
        void deleteCommentSuccess();

    signals:
        void newComment(const QString & text, bool makePublic, const QString & parent);
        void publishMyComment(const QString & uri, bool publish);
        void deleteMyComment(const QString & uri);

    protected:

        // Scroll area and layout
        QScrollArea * _centralScrollArea;
        QWidget * _scrollWidget;
        QWidget * _centralWidget;
        QVBoxLayout * _layout;
        QVBoxLayout * _outerLayout;
        QStatusBar * _statusBar;

        // Comment Editors and widgets
        CommentEditor * _editorReply;
        CommentEditor * _editorNewComment;
        QPushButton * _addButton;

        // Comment populating
        QMap< Comment *, AbstractCommentData * > _commentToDataMap;
        QMap< QString , Comment * > _idToCommentMap;

        MessageLabel * _messageLabel;
        QSpacerItem * _spacer;
        Spinner * _spinner;
        AbstractCommentData * _activeParent;

        bool _addButtonAllowed;
    };

} // namespace Utopia

#endif
