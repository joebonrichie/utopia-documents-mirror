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

#ifndef COMMENT_H
#define COMMENT_H

#include <utopia2/auth/qt/config.h>
#include <utopia2/qt/abstractcommentdata.h>
#include <utopia2/qt/fieldeditor.h>

#include <QWidget>
#include <QString>
#include <QImage>

typedef QMap< QString, QString > QStringMap;

class QPushButton;
class QHBoxLayout;
class QVBoxLayout;

namespace Kend
{
    class User;
    class UserLabel;
}

namespace Utopia
{
    class FieldEditor;
    class RoundedPixmap;

    class LIBUTOPIA_AUTH_QT_API Comment : public QWidget
    {

        Q_OBJECT

    public:
        Comment(Comment * parentComment, AbstractCommentData * data, bool editable = false, bool reversed = false);
        Comment * parentComment() const;
        int indentLevel() const;
        void populate();
        QDateTime date();
        AbstractCommentData * commentData();
        bool isDraft();

        void refresh();

    protected slots:
        // Required for FieldEditor
        void editField(FieldEditor *);
        void report();
        void updateComment();
        void setDetails(QStringMap info);
        void setImage(QImage image);

        void deleteMe();
        void publishMe();

    protected:
        void mousePressEvent(QMouseEvent * event);
        void getData(); // Called by constructor
        virtual void drawBubble(); // Called by constructor

    signals:
        void updateMePlease();
        void replyPressed();

        void publishPressed(QString uid);
        void deletePressed(QString uid);

    protected:
        Comment * _parentComment;
        QString _text;
        QString _username;
        QString _date;
        bool _reversed;

        //FieldEditor * _label;
        // Required for FieldEditor
        TextEdit * _editor;
        FieldEditor * _highlightWidget;
        QPushButton * _replyButton;

        AbstractCommentData * _data;

        // GUI
        QHBoxLayout * commentLayout; // The main layout
        QVBoxLayout * bubbleLayout; // The layout of the bubble area
        QHBoxLayout * toplayout; // The top part of the bubble
        QWidget * bubble; // The speech bubble
        QLabel * datelabel; // The date label
        QLabel * _label;
        QPushButton * _deleteButton;
        QPushButton * _publishButton;
        QPushButton * reportButton; // The report button

        Kend::UserLabel * _usernameLabel;
        Kend::UserLabel * _avatarLabel;
    };

} // Namespace Utopia

#endif // COMMENT_H
