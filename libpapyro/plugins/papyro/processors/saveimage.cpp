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

#include "saveimage.h"

#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>
#include <utopia2/qt/imageformatmanager.h>
#include <utopia2/qt/imagepreview.h>
#include <utopia2/qt/hidpi.h>

#include <QLabel>
#include <QEvent>
#include <QStackedLayout>
#include <QToolButton>

#include <QtDebug>




namespace Papyro
{

    ContextPreview::ContextPreview(QWidget * parent, const QPixmap & pixmap, const QVariantMap & params)
        : QWidget(parent, Qt::Window), pixmap(pixmap), params(params)
    {
        Utopia::ImagePreview * preview = new Utopia::ImagePreview();
        preview->setPixmap(pixmap);

        QVBoxLayout * layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(preview);

        sliver = new QFrame(this);
        sliver->setObjectName("sliver");
        QVBoxLayout * sliverLayout = new QVBoxLayout(sliver);
        sliverLayout->addStretch();

        QToolButton * saveContextButton = new QToolButton;
        saveContextButton->resize(saveContextButton->sizeHint() * Utopia::hiDPIScaling());
        saveContextButton->setIcon(QIcon(":/processors/save-context.png"));
        saveContextButton->setToolTip("Save image as...");
        connect(saveContextButton, SIGNAL(clicked()), this, SLOT(onSaveContextButtonClicked()));
        sliverLayout->addWidget(saveContextButton);

        QToolButton * showContextButton = new QToolButton;
        showContextButton->resize(saveContextButton->sizeHint() * Utopia::hiDPIScaling());
        showContextButton->setIcon(QIcon(":/processors/show-context.png"));
        showContextButton->setToolTip("Show in article");
        connect(showContextButton, SIGNAL(clicked()), this, SLOT(onShowContextButtonClicked()));
        sliverLayout->addWidget(showContextButton);

        const int margin = 5 * Utopia::hiDPIScaling();
        sliverLayout->setContentsMargins(margin, margin, margin, margin);
        sliver->adjustSize();

        QRect sliverGeometry(-sliver->width(), 0, sliver->width(), height());
        sliver->setGeometry(sliverGeometry);

        slidingAnimation.setTargetObject(sliver);
        slidingAnimation.setPropertyName("geometry");
        slidingAnimation.setDuration(150);
        slidingAnimation.setEasingCurve(QEasingCurve::InOutSine);
        slidingAnimation.setStartValue(sliverGeometry);
        sliverGeometry.moveLeft(0);
        slidingAnimation.setEndValue(sliverGeometry);
        slidingAnimation.setDirection(QAbstractAnimation::Forward);
    }

    ContextPreview::~ContextPreview()
    {}

    void ContextPreview::enterEvent(QEvent * event)
    {
        bool interrupting = (slidingAnimation.state() == QAbstractAnimation::Running);
        if (interrupting) {
            slidingAnimation.pause();
        }
        slidingAnimation.setDirection(QAbstractAnimation::Forward);
        if (interrupting) {
            slidingAnimation.resume();
        } else {
            slidingAnimation.start();
        }
    }

    void ContextPreview::leaveEvent(QEvent * event)
    {
        bool interrupting = (slidingAnimation.state() == QAbstractAnimation::Running);
        if (interrupting) {
            slidingAnimation.pause();
        }
        slidingAnimation.setDirection(QAbstractAnimation::Backward);
        if (interrupting) {
            slidingAnimation.resume();
        } else {
            slidingAnimation.start();
        }
    }

    void ContextPreview::onSaveContextButtonClicked()
    {
        Utopia::ImageFormatManager::saveImageFile(this, "Save Image As...", pixmap, "Image Copy");
    }

    void ContextPreview::onShowContextButtonClicked()
    {
        emit showPage(params);
    }

    void ContextPreview::resizeEvent(QResizeEvent * event)
    {
        sliver->setFixedHeight(height());
        QRect startGeometry = slidingAnimation.startValue().toRect();
        startGeometry.setHeight(height());
        slidingAnimation.setStartValue(startGeometry);
        QRect endGeometry = slidingAnimation.endValue().toRect();
        endGeometry.setHeight(height());
        slidingAnimation.setEndValue(endGeometry);
    }




    /////////////////////////////////////////////////////////////////////////////////////////
    /// SaveImageProcessor

    int SaveImageProcessor::category() const
    {
        return -10; // Try to be the first and most default section
    }

    void SaveImageProcessor::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
    {
        // Only save if there's an image under the cursor
        if (const Spine::Image * i = cursor->image()) {
            // Create QPixmap of the Spine image
            QImage image(qImageFromSpineImage(i));
            Utopia::ImageFormatManager::saveImageFile(0, "Save Image As...", QPixmap::fromImage(image), "Image Copy");
        }
    }

    QString SaveImageProcessor::title() const
    {
        return "Save Image As...";
    }

    int SaveImageProcessor::weight() const
    {
        return 10;
    }




    /////////////////////////////////////////////////////////////////////////////////////////
    /// PopOutImageProcessor

    int PopOutImageProcessor::category() const
    {
        return -10; // Try to be the first and most default section
    }

    void PopOutImageProcessor::processSelection(Spine::DocumentHandle document, Spine::CursorHandle cursor, const QPoint & globalPos)
    {
        // If there's an area selection, make an image out of that
        // otherwise take it from the image under the cursor.
        QPixmap pixmap;
        QRectF rect;
        int pageNumber = 1;
        if (ImagingProcessorFactory::hasAreaSelection(document, cursor)) {
            // This page
            const Spine::Page * page = cursor->page();
            // First area grabbed as an image
            Spine::AreaSet areas = document->areaSelection();
            foreach (const Spine::Area & area, areas) {
                if (area.page == page->pageNumber()) {
                    Spine::Image spineImage = page->renderArea(area.boundingBox, 400.0);
                    Spine::BoundingBox bb = area.boundingBox;
                    pixmap = QPixmap::fromImage(qImageFromSpineImage(&spineImage));
                    rect = QRect(bb.x1, bb.y1, bb.width(), bb.height());
                    pageNumber = area.page;
                    break;
                }
            }
        } else if (const Spine::Image * i = cursor->image()) {
            // Create QPixmap of the Spine image
            Spine::BoundingBox bb = i->boundingBox();
            pixmap = QPixmap::fromImage(qImageFromSpineImage(i));
            rect = QRect(bb.x1, bb.y1, bb.width(), bb.height());
            pageNumber = cursor->page()->pageNumber();
        }
        if (!pixmap.isNull()) { // This should ALWAYS be the case
            if (PapyroWindow * window = PapyroWindow::currentWindow()) {
                if (PapyroTab * tab = window->currentTab()) {
                    QVariantMap params;
                    params["page"] = pageNumber;
                    params["rect"] = rect;

                    ContextPreview * popout = new ContextPreview(tab->documentView(), pixmap, params);
                    QSize size = pixmap.size();
                    size.scale(QSize(500, 300) * Utopia::hiDPIScaling(), Qt::KeepAspectRatio);
                    popout->setWindowTitle("Pop-out Snapshot");
                    popout->resize(size);
                    popout->setMinimumSize(100, 100);
                    popout->show();

                    QObject::connect(popout, SIGNAL(showPage(const QVariantMap &)),
                                     tab->documentView(), SLOT(showPage(const QVariantMap &)));
                }
            }
        }
    }

    QString PopOutImageProcessor::title() const
    {
        return "Pop Out Snapshot";
    }

    int PopOutImageProcessor::weight() const
    {
        return 10;
    }




    /////////////////////////////////////////////////////////////////////////////////////////
    /// ImagingProcessorFactory

    QList< boost::shared_ptr< SelectionProcessor > > ImagingProcessorFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
    {
        QList< boost::shared_ptr< SelectionProcessor > > list;
        if (cursor->image() != 0 || ImagingProcessorFactory::hasAreaSelection(document, cursor)) {
            list << boost::shared_ptr< SelectionProcessor >(new PopOutImageProcessor);
        }
        if (cursor->image() != 0) {
            list << boost::shared_ptr< SelectionProcessor >(new SaveImageProcessor);
        }
        return list;
    }

}  // namespace Papyro

