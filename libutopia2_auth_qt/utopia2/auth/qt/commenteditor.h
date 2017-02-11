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

#ifndef COMMENTEDITOR_H
#define COMMENTEDITOR_H

#include <utopia2/auth/qt/config.h>
#include <utopia2/auth/servicemanagernotifier.h>
#include <utopia2/qt/uimanager.h>

#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <QTextEdit>

class QStackedLayout;

namespace Utopia
{
    class Comment;
    class RoundedPixmap;

    class LIBUTOPIA_AUTH_QT_API CommentEditor : public QWidget
    {

        Q_OBJECT

    public:
        CommentEditor(Comment * parentComment = 0);
        void clear();
        void setParentComment(Comment * parentComment);
        void setStandalone();
        QString text();

    public slots:
        void show();

    signals:
        void cancelled();
        void submitted();
        void saved();

    protected slots:
        void onValidityChanged(bool valid);

    protected:
        void paintEvent(QPaintEvent * event);

    protected:
        boost::shared_ptr< UIManager > uiManager;

        Comment * _parentComment;

        QStackedLayout * _stackedLayout;
        QTextEdit * _editor;
        QLabel * _usernameLabel;
        QPushButton * _submit;
        QPushButton * _cancel;
        QPushButton * _save;

        Kend::ServiceManagerNotifier * validServiceNotifier;

        bool _standalone;
    };

} // Namespace Utopia

#endif // COMMENTEDITOR_H
