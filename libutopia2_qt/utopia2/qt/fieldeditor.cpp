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

#include <utopia2/qt/fieldeditor.h>

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QTextEdit>
#include <QApplication>

namespace Utopia
{

    FieldEditor::FieldEditor(QWidget * parent, QString defaultText) : QLabel(parent)
    {
        this->setMargin(5);
        this->setMouseTracking(true);
        this->setMinimumWidth(20);
        this->_highlight = false;
        this->_defaultText = defaultText;
        this->_editable = false;
        this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

        this->setObjectName("empty");

        this->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        this->setOpenExternalLinks(true);

        connect(this, SIGNAL(startEdit(FieldEditor *)), this->parent(), SLOT(editField(FieldEditor *)));
    }

    void FieldEditor::paintEvent(QPaintEvent * event)
    {
        if (this->_highlight == true)
        {
            QPainter * painter = new QPainter(this);
            painter->setRenderHint(QPainter::TextAntialiasing, true);
            painter->setPen(QColor(0,0,0,0));
            painter->setBrush(QColor(207,217,227,150));
            painter->drawRoundedRect(this->rect().adjusted(0,0,-1,-1), 5,5);
            delete painter;
        }

        this->QLabel::paintEvent(event);
    }

    void FieldEditor::enterEvent(QEvent *)
    {
        if (_editable)
        {
            this->_highlight = true;
        }
        this->update();
    }

    bool FieldEditor::isEmpty()
    {
        if (text() == _defaultText)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void FieldEditor::lostFocus()
    {
        qDebug() << "emitting stopEdit()";
        emit stopEdit();
    }

    void FieldEditor::leaveEvent(QEvent *)
    {
        this->_highlight = false;
        this->update();
    }

    void FieldEditor::mousePressEvent(QMouseEvent *)
    {
        emit startEdit(this);
    }

    void FieldEditor::setEditorWordWrap(bool wordWrap)
    {
        _wordwrap = wordWrap;
    }

    void FieldEditor::setEditable(bool editable)
    {
        _editable = editable;
        if (!editable)
        {
            disconnect(this, SIGNAL(startEdit(FieldEditor *)), this->parent(), SLOT(editField(FieldEditor *)));
        }
        else
        {
            connect(this, SIGNAL(startEdit(FieldEditor *)), this->parent(), SLOT(editField(FieldEditor *)));
        }
    }

    bool FieldEditor::editorWordWrap()
    {
        return _wordwrap;
    }

    void FieldEditor::setText(const QString & string)
    {
        if (string.isEmpty())
        {
            this->_defaultValue = true;
            this->setObjectName("empty");
            QLabel::setText(this->_defaultText);
        }
        else
        {
            this->_defaultValue = false;
            this->setObjectName("inuse");
            QLabel::setText(string);
        }
        this->setStyle(QApplication::style());
    }

    TextEdit::TextEdit(FieldEditor * owner, QWidget * parent) : QTextEdit(parent)
    {
        _owner = owner;
    }

    void TextEdit::commit()
    {
        qDebug() << "textedit commit";
        if (_owner)
        {
            _owner->lostFocus();
        }
    }

    void TextEdit::keyPressEvent(QKeyEvent * event)
    {
        if (event->key() == Qt::Key_Return)
        {
            qDebug() << "Enter";
            _owner->lostFocus();
        }
        else if (event->key() == Qt::Key_Tab)
        {
            qDebug() << "Tab";
            _owner->lostFocus();
        }
        else
        {
            QTextEdit::keyPressEvent(event);
        }

    }

} // namespace Utopia
