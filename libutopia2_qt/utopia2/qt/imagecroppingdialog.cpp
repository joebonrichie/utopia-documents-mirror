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

#include <utopia2/qt/imagecroppingdialog.h>
#include <utopia2/qt/thumbnailchooser.h>

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Utopia
{
    ImageCroppingDialog::ImageCroppingDialog(QPixmap source, QWidget * parent)
        : BubbleDialog(parent)
    {
        // General setup
        this->resize(450, 250);
        this->setWindowTitle("Adjust Your Image");

        _thumbnailChooser = new ThumbnailChooser;
        _thumbnailChooser->setSmoothUpdates(true);
        _thumbnailChooser->setSourceImage(source);
        _thumbnailChooser->setThumbnailSize(QSize(128, 128));
        _thumbnailChooser->setCropRect(QRectF(QPointF(0, 0), source.size()));
        QLabel * preview = _thumbnailChooser->thumbnailPreview();
        preview->setObjectName("thumbnail_preview");
        preview->setFixedSize(QSize(128, 128));

        QHBoxLayout * hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(12, 12, 12, 12);
        hLayout->setSpacing(5);
        hLayout->addWidget(_thumbnailChooser, 1);
        hLayout->addWidget((QWidget *) preview, 0, Qt::AlignCenter);

        QPushButton * rejectButton = new QPushButton();
        rejectButton->setText("Cancel");
        connect(rejectButton, SIGNAL(clicked()), this, SLOT(reject()));

        QPushButton * acceptButton = new QPushButton();
        acceptButton->setText("Use Image");
        connect(acceptButton, SIGNAL(clicked()), this, SLOT(doAccept()));

        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->addWidget(rejectButton);
        buttonLayout->addStretch(0);
        buttonLayout->addWidget(acceptButton);

        QVBoxLayout * vLayout = new QVBoxLayout(this);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        vLayout->addLayout(hLayout);
        vLayout->addLayout(buttonLayout);
    }

    void ImageCroppingDialog::doAccept()
    {
        this->resultPixmap = _thumbnailChooser->thumbnail(true);
        accept();
    }

    QPixmap ImageCroppingDialog::pixmap()
    {
        return this->resultPixmap;
    }

} // namespace Utopia
