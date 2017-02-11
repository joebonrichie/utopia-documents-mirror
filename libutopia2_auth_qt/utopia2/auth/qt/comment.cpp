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

#include <utopia2/auth/qt/comment.h>
#include <utopia2/qt/bubble.h>
#include <utopia2/qt/roundedpixmap.h>
#include <utopia2/qt/speechbubble.h>
#include <utopia2/qt/thoughtbubble.h>

#include <utopia2/auth/servicemanager.h>
#include <utopia2/auth/user.h>
#include <utopia2/auth/qt/userlabel.h>

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QScrollArea>
#include <QScrollBar>
#include <QPixmap>
#include <QImage>
#include <QThreadPool>

namespace Utopia
{

    Comment::Comment(Comment * parentComment, AbstractCommentData * data, bool editable, bool reversed)
        : QWidget(), _parentComment(parentComment), _reversed(reversed), _data(data), reportButton(0)
    {
        _editor = 0;
        _highlightWidget = 0;

        Kend::Service * service = 0;
        boost::shared_ptr< Kend::ServiceManager > serviceManager(Kend::ServiceManager::instance());
        if (serviceManager->count() == 1) {
            service = serviceManager->serviceAt(0);
        }

        Kend::User * user = 0;
        if (!_data->author().isEmpty()) {
            user = new Kend::User(service, _data->author());
        }

        // Layout stuff
        this->setContentsMargins(48 * this->indentLevel(),0,0,0);
        this->commentLayout = new QHBoxLayout;
        this->setLayout(this->commentLayout);
        this->commentLayout->setSpacing(2);
        this->commentLayout->setContentsMargins(10,5,10,5);
        this->commentLayout->setDirection(this->_reversed? QBoxLayout::RightToLeft: QBoxLayout::LeftToRight);

        // User picture
        this->_avatarLabel = new Kend::UserLabel(user, Kend::UserLabel::Avatar);
        this->_avatarLabel->setRoundedCorners(true);
        this->_avatarLabel->setFixedSize(48, 48);
        this->_avatarLabel->setScaledContents(true);
        this->commentLayout->addWidget(this->_avatarLabel);
        this->commentLayout->setAlignment(this->_avatarLabel, Qt::AlignTop);

        // Comment data
        this->getData();

        // Speech bubble
        this->drawBubble();
    }

    void Comment::drawBubble()
    {
        QString styleSheet;
        if ( this->_data->isPublic() )
        {
            this->bubble = new SpeechBubble(0,this->_reversed);
            styleSheet = "QLabel { background-color : rgb(120,120,120); }";
        }
        else
        {
            this->bubble = new ThoughtBubble(0, this->_reversed);
            styleSheet = "QLabel { background-color : rgb(80,80,80); }";
        }

        // The bubble layout
        this->bubbleLayout = new QVBoxLayout;
        if (!this->_reversed)   { bubbleLayout->setContentsMargins(15,5,5,5); }
        else                    { bubbleLayout->setContentsMargins(5,5,15,5); }
        this->bubble->setLayout(bubbleLayout);

        // The label displaying the comment's text [!] Could try FieldEditor instead?
        this->_label = new QLabel(this);
        this->_label->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        this->_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        this->_label->setOpenExternalLinks(true);
        this->_label->setWordWrap(true);
        this->_label->setTextFormat(Qt::RichText);
        this->_label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        QString richText = this->_text.replace("&", "&amp;").replace("<", "&lt;");
        static QRegExp urlRegExp("(\\b(?:(?:https?://|www\\d{0,3}[.]|[a-z0-9.\\-]+[.][a-z]{2,4}/)(?:[^\\s()<>]+|\\((?:[^\\s()<>]+|(?:\\([^\\s()<>]+\\)))*\\))+(?:\\((?:[^\\s()<>]+|(?:\\([^\\s()<>]+\\)))*\\)|[^\\s`!()\\[\\]{};:'\".,<>?])))");
        static QString htmlTpl(
            "<html>"
              "<head>"
                "<style type=text/css>"
                  "a:link { text-decoration: none; color: #6df; } "
                  "a:hover { text-decoration: underline; } "
                  ".right { text-align: right; }"
                "</style>"
              "</head>"
              "<body>"
                "%1"
              "</body>"
            "</html>"
            );
        richText.replace(urlRegExp, "<a href=\"\\1\">\\1</a>");
        this->_label->setText(htmlTpl.arg(richText));

        // For stuff at the top of the bubble
        this->toplayout = new QHBoxLayout;
        toplayout->setSpacing(4);

        // The user name
        Kend::Service * service = 0;
        boost::shared_ptr< Kend::ServiceManager > serviceManager(Kend::ServiceManager::instance());
        if (serviceManager->count() == 1) {
            service = serviceManager->serviceAt(0);
        }
        Kend::User * user = 0;
        if (!_data->author().isEmpty()) {
            user = new Kend::User(service, _data->author());
        }
        this->_usernameLabel = new Kend::UserLabel(user, Kend::UserLabel::DisplayName);

        // The date
        this->datelabel = new QLabel;
        this->datelabel->setObjectName("date");
        this->datelabel->setText(this->_date);

        this->_usernameLabel->setStyleSheet(styleSheet);
        this->datelabel->setStyleSheet(styleSheet);
        this->_label->setStyleSheet(styleSheet);

        // Populating the top layout
        toplayout->addWidget(this->_usernameLabel);
        toplayout->addStretch(1);
        toplayout->addWidget(this->datelabel);

        // Adding each layer to the bubble layout
        bubbleLayout->addLayout(toplayout);
        bubbleLayout->addWidget(this->_label);

        // Adding the bubble to the main layout
        this->commentLayout->addWidget(this->bubble);
        this->commentLayout->setAlignment(this->bubble, Qt::AlignTop);

        // Finish bubble... ////////////////////////////////////////////////////////////////////////

        // No report button for unpublished comments... You're not going to report your own comments, are you?!
        if (false && this->_data->isPublic())
        {
            // The report button
            this->reportButton = new QPushButton;
            this->reportButton->setFixedSize(16,16);
            this->reportButton->setObjectName("report");
            connect(this->reportButton, SIGNAL(clicked()), this, SLOT(report()));
            toplayout->addWidget(this->reportButton);
        }

        QVariantMap defaults(Utopia::defaults());
        QString method = defaults.value("service_method").toString();
        if (method != "prevent") {
            // The bottom layout (for action buttons) [!] cf ACLs! (+ interactivity option...)
            QHBoxLayout * bottomlayout = new QHBoxLayout;

            // The various action buttons (reply, edit, delete => cf ACLs!)
            bool reply = true; // [!] // Does it make sense when unpublished? Other name??
            if (reply)
            {
                _replyButton = new QPushButton(); // [!]
                _replyButton->setText("Reply");
                connect(_replyButton, SIGNAL(clicked()), this, SIGNAL(replyPressed()));
                // [!] Reply may be deactivated if read only mode...? Or replaced with "Note to self"
                // [!] May also want an annotate button to reply only in private personal context?
                bottomlayout->addWidget(_replyButton);
            }

            bool del = !this->_data->isPublic(); // Always true if unpublished...
            if (del)
            {
                _deleteButton = new QPushButton();
                _deleteButton->setText("Delete");
                connect(_deleteButton, SIGNAL(clicked()), this, SLOT(deleteMe()));
                bottomlayout->addWidget(_deleteButton);
            }

            bool publish = !this->_data->isPublic(); // Only available on unpublished comments
            if (publish)
            {
                _publishButton = new QPushButton();
                _publishButton->setText("Publish");
                connect(_publishButton, SIGNAL(clicked()), this, SLOT(publishMe()));
                bottomlayout->addWidget(_publishButton);
            }

            bottomlayout->addStretch();
            if (reply || del || publish) { bubbleLayout->addLayout(bottomlayout); }
            else { delete bottomlayout; }
        }
    }

    void Comment::getData()
    {
        if (this->_data)
        {
            _text = this->_data->text();
            _date = this->_data->dateSince(QDateTime::currentDateTime(), this->_data->date());
        }
        else { _text = ""; _date = ""; }
    }

    bool Comment::isDraft()
    {
        if (!this->_data) return true; // really should raise an exception [!]
        return !this->_data->isPublic();
    }

    AbstractCommentData * Comment::commentData()
    {
        return _data;
    }

    QDateTime Comment::date()
    {
        return _data->date();
    }

    void Comment::refresh()
    {
        if (this->bubble) delete this->bubble; // Deleting old GUI
        this->getData(); // Refreshing the data
        this->drawBubble(); // Drawing new GUI
     }

    void Comment::editField(FieldEditor * editorWidget)
    {
        qDebug() << "editField()";

        if (_editor != 0)
        {
            qDebug() << "deleting editor";
            delete _editor;
        }

        _editor = new TextEdit(editorWidget, (QWidget *) editorWidget->parent());
        _editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        _editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//        if  (!editorWidget->editorWordWrap())
        {
//            _editor->setLineWrapMode(QTextEdit::NoWrap);
        }
        _editor->setText(editorWidget->text());
        _editor->setFont(editorWidget->font());
        _editor->setFrameStyle(QFrame::NoFrame);
        _editor->setGeometry(QRect(editorWidget->pos(), QSize(editorWidget->rect().width(), editorWidget->rect().height())).adjusted(1,1,0,0));
        _editor->setMinimumHeight(100);
        _editor->setFocusPolicy(Qt::ClickFocus);

        _editor->selectAll();
        _editor->setFocus();
        _editor->show();
        _editor->raise();

        _highlightWidget = editorWidget;

        this->adjustSize();
        this->update();
    }

    int Comment::indentLevel() const
    {
        if (this->parentComment() == 0) { return 0; }
        else { return this->_parentComment->indentLevel() + 1; }
    }

    void Comment::mousePressEvent(QMouseEvent * event)
    {
        if ((_editor != 0) && (_highlightWidget != 0))
        {
            _highlightWidget->lostFocus();
        }
        QWidget::mousePressEvent(event);
        this->update();
    }

    void Comment::publishMe() { emit publishPressed(this->_data->id()); }
    void Comment::deleteMe() { emit deletePressed(this->_data->id()); }

    void Comment::report()
    {
        BubbleDialog * sureDialog = new BubbleDialog;
        sureDialog->setWindowTitle("Report Comment");
        QGridLayout * layout = new QGridLayout(sureDialog);
        QLabel * label = new QLabel;
        label->setWordWrap(true);
        label->setText("Are you sure you would like to report this comment to the moderating team?");
        QPushButton * acceptButton = new QPushButton();
        acceptButton->setText("Report");
        connect(acceptButton, SIGNAL(clicked()), sureDialog, SLOT(accept()));
        QPushButton * cancelButton = new QPushButton();
        cancelButton->setText("Cancel");
        connect(cancelButton, SIGNAL(clicked()), sureDialog, SLOT(reject()));
        layout->addWidget(label, 0, 0, 1, 2, Qt::AlignCenter);
        layout->addWidget(cancelButton, 1, 0, Qt::AlignLeft);
        layout->addWidget(acceptButton, 1, 1, Qt::AlignRight);
        sureDialog->adjustSize();
        sureDialog->setFixedWidth(300);

        if (sureDialog->exec())
        {
            // FIXME reporting annotations functionality goes here!

            BubbleDialog * dialog = new BubbleDialog;
            dialog->setWindowTitle("Comment Reported");
            QGridLayout * layout = new QGridLayout(dialog);
            QLabel * label = new QLabel;
            label->setWordWrap(true);
            label->setText("Thank you. This comment has been reported to the moderating team.");
            QPushButton * acceptButton = new QPushButton();
            acceptButton->setText("OK");
            connect(acceptButton, SIGNAL(clicked()), dialog, SLOT(accept()));
            layout->addWidget(label, 0, 0, 1, 2, Qt::AlignCenter);
            layout->addWidget(acceptButton, 1, 1, Qt::AlignRight);
            dialog->adjustSize();
            dialog->setFixedWidth(300);
            dialog->exec();
        }
    }

    Comment * Comment::parentComment() const
    {
        return this->_parentComment;
    }

    void Comment::setImage(QImage image)
    {
    }

    void Comment::setDetails(QStringMap info)
    {
    }

    void Comment::updateComment()
    {
        qDebug() << "update comment";
        qDebug() << _label->text();
        qDebug() << _editor->toPlainText();
        if (_label->text() != _editor->toPlainText())
        {
            emit updateMePlease();
            qDebug() << "update please";
        }
        if (_editor)
        {
            qDebug() << "removing editor";
            delete _editor;
            _editor = 0;
        }
        _highlightWidget = 0;
        this->update();
    }


} // namespace Utopia
