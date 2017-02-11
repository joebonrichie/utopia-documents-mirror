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

#include <utopia2/auth/qt/commenteditor.h>
#include <utopia2/auth/qt/comment.h>
#include <utopia2/qt/roundedpixmap.h>


#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalMapper>
#include <QStackedLayout>
#include <QPixmap>
#include <QPainter>
#include <QImage>

namespace Utopia
{

    CommentEditor::CommentEditor(Comment * parentComment)
        : QWidget(), _parentComment(parentComment)
    {
        _standalone = false;

        validServiceNotifier = new Kend::ServiceManagerNotifier(Kend::ServiceManager::FilterLoggedIn, this);
        connect(validServiceNotifier, SIGNAL(validityChanged(bool)), this, SLOT(onValidityChanged(bool)));

        if (_parentComment) { this->setContentsMargins(48 * _parentComment->indentLevel(), 0, 0, 0); }
        else { this->setContentsMargins(0, 0, 0, 0); }

        QVBoxLayout * layout = new QVBoxLayout;
        layout->setContentsMargins(10, 20, 10, 10);
        this->setLayout(layout);

        QHBoxLayout * buttonLayout = new QHBoxLayout;

        _cancel = new QPushButton();
        _cancel->setText("Cancel");
        connect(_cancel, SIGNAL(clicked()), this, SIGNAL(cancelled()));
        buttonLayout->addWidget(_cancel);

        buttonLayout->addStretch();

        _save = new QPushButton();
        _save->setText("Save as Draft");
        connect(_save, SIGNAL(clicked()), this, SIGNAL(saved()));
        buttonLayout->addWidget(_save);
        _save->setEnabled(validServiceNotifier->isValid());
        connect(validServiceNotifier, SIGNAL(validityChanged(bool)), _save, SLOT(setEnabled(bool)));

        _submit = new QPushButton();
        _submit->setText("Submit Comment");
        connect(_submit, SIGNAL(clicked()), this, SIGNAL(submitted()));
        buttonLayout->addWidget(_submit);
        _submit->setEnabled(validServiceNotifier->isValid());
        connect(validServiceNotifier, SIGNAL(validityChanged(bool)), _submit, SLOT(setEnabled(bool)));

        // Doesn't really work right now, because of the way conversation works [!]
        if (this->_parentComment) if (this->_parentComment->isDraft()) this->_submit->hide();

        _stackedLayout = new QStackedLayout;
        _editor = new QTextEdit;

        QFrame * authPane = new QFrame;
        QVBoxLayout * vLayout = new QVBoxLayout(authPane);

        QLabel * authenticateLabel = new QLabel("<em style='color: #c88; font-weight: bold'>You must authenticate<br/>before commenting...</em>");
        authenticateLabel->setAlignment(Qt::AlignCenter);
        connect(validServiceNotifier, SIGNAL(validityChanged(bool)), authenticateLabel, SLOT(setHidden(bool)));
        authenticateLabel->setHidden(validServiceNotifier->isValid());
        vLayout->addStretch(1);
        vLayout->addWidget(authenticateLabel, 0, Qt::AlignCenter);
        authenticateLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));

        QPushButton * authButton = new QPushButton;
        authButton->setText("Authenticate");
        connect(validServiceNotifier, SIGNAL(validityChanged(bool)), authButton, SLOT(setHidden(bool)));
        authButton->setHidden(validServiceNotifier->isValid());
        uiManager = UIManager::instance();
        QSignalMapper * mapper = new QSignalMapper(this);
        mapper->setMapping(authButton, "Accounts");
        connect(mapper, SIGNAL(mapped(const QString &)), uiManager.get(), SLOT(showPreferences(const QString &)));
        connect(authButton, SIGNAL(clicked()), mapper, SLOT(map()));
        authButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
        vLayout->addWidget(authButton, 0, Qt::AlignCenter);
        vLayout->addStretch(1);

        _stackedLayout->addWidget(_editor);
        _stackedLayout->addWidget(authPane);
        onValidityChanged(validServiceNotifier->isValid());

        layout->addLayout(_stackedLayout);
        layout->addLayout(buttonLayout);
    }

    void CommentEditor::onValidityChanged(bool valid)
    {
        _stackedLayout->setCurrentIndex(valid ? 0 : 1);
    }

    void CommentEditor::clear() { _editor->clear(); }

    void CommentEditor::paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);

        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRect rect =  this->contentsRect().adjusted(1,1,-1,-1);

        int pipWidth = 10;
        int pipOffset = (rect.width() / 2) - (pipWidth / 2);
        QPainterPath bubbleRect;

        if (!_standalone)
        {   bubbleRect.addRoundedRect(QRect(rect.topLeft()+QPoint(0, pipWidth+2), rect.bottomRight()),5,5); }
        else { bubbleRect.addRoundedRect(QRect(rect.topLeft(), rect.bottomRight()),5,5); }

        QPainterPath bubblePip;
        QPoint start;

        if (!_standalone)
        {
            start = rect.topLeft()+QPoint(pipOffset, 2);
            bubblePip.moveTo(start);
            bubblePip.lineTo(start+QPoint(pipWidth,pipWidth+1));
            bubblePip.lineTo(start+QPoint(-pipWidth,pipWidth+1));
            bubblePip.lineTo(start);

            QPainterPath bubble = bubblePip.united(bubbleRect);
            painter.setBrush(QColor(80,80,80));
            painter.setPen(Qt::black);
            painter.drawPath(bubble);
        }
        else
        {
            painter.setBrush(QColor(80,80,80));
            painter.setPen(Qt::black);
            painter.drawPath(bubbleRect);
        }

        QWidget::paintEvent(event);
    }

    void CommentEditor::setParentComment(Comment * parentComment)
    {
        _parentComment = parentComment;

        if (_parentComment)
        {   this->setContentsMargins(48 * (_parentComment->indentLevel() + 1) + 70, 0, 10, 0); }
        else { this->setContentsMargins(0, 0, 0, 0); }
        update();
    }

    void CommentEditor::setStandalone() { _standalone = true; }

    void CommentEditor::show()
    {
        qDebug() << "focus";
        this->setFocus(Qt::OtherFocusReason);
        QWidget::show();
    }

    QString CommentEditor::text() { return _editor->toPlainText(); }

} // namespace Utopia
