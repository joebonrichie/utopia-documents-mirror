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

#include "papyropreview.h"

#include <QLabel>
#include <QHBoxLayout>

#include <utopia2/node.h>
#include <utopia2/qt/fullscreenoverlay.h>
#include <papyro/documentfactory.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

PapyroPreview::PapyroPreview(QWidget * parent, Qt::WindowFlags f)
    : AbstractPreview(parent, f)
{
    QVBoxLayout *layout= new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);


    documentView= new Papyro::DocumentView; //(NULL, QColor(0,0,40,200));
//      QWidget * w = new Utopia::FullScreenOverlay;
//      QVBoxLayout *layout2= new QVBoxLayout(w);
//      layout2->setContentsMargins(50, 10, 150, 50);
//      layout2->setSpacing(0);
//      layout2->addWidget(documentView);
//      documentView->setContentsMargins(0, 0, 0, 0);
//      documentView->setSpacing(0);
//      w->showFullScreen();

    layout->addWidget(documentView);
}

PapyroPreview::~PapyroPreview()
{}

Utopia::Node * PapyroPreview::type() const
{
    return Utopia::UtopiaDomain.term("Document");
}

void PapyroPreview::modelSet()
{
    if (this->model())
    {
        Papyro::Document * document = PAPYRO::DocumentFactory::load(this->model());

//              this->_hud->setDocument(document,0);

        this->documentView->setDocument(document,0);
        documentView->setZoomMode(Papyro::DocumentView::FitToWindow);
//              documentView->setPageMode(Papyro::DocumentView::TwoUp);
//              documentView->setSpineSpacing(0);
        documentView->setPageDecorations(Papyro::DocumentView::Shadows);

//              this->_hudWidget->show();

        this->update();
    }
}
