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

#include <utopia2/qt/tearout.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QImage>

namespace Utopia
{

    Tearout::Tearout(QString text)
        : _text(text)
    {
        _label = new QLabel(text);
        _label->setWordWrap(true);
        QVBoxLayout * layout = new QVBoxLayout;
        layout->setContentsMargins(20, 10, 20, 10);
        layout->addWidget(_label, Qt::AlignCenter);
        this->setLayout(layout);
        this->setContentsMargins(10, 10, 10, 10);
        this->setMaximumWidth(512);
        this->setMaximumHeight(512);
        this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

        QFont font = this->font();
        font.setStyleHint(QFont::Serif);
        this->setFont(font);
    }

    void Tearout::paintEvent(QPaintEvent * event)
    {
        QPainter painter(this);

        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        QRect rect = this->rect().adjusted(0, 0, 0, 0);
        QPixmap background(":images/torn-paper.png");

        painter.drawPixmap(rect, background, background.rect());
    }

    void Tearout::resizeEvent(QResizeEvent * event)
    {
        QFont font = this->font();
        QFontMetrics fm(font);

        _label->setText(fm.elidedText(_text, Qt::ElideRight, this->width() * ((this->height() - 10) / (fm.lineSpacing()))));
    }
/*
  QSize Tearout::sizeHint() const
  {
  qDebug() << "sizehint";
  qDebug() << _label->sizeHint() + QSize(10, 10);
  return _label->size() + QSize(10, 10);
  }
*/
} // namespace Utopia
