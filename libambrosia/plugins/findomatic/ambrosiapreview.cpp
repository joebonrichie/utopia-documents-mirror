/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include "ambrosiapreview.h"

#include <QLabel>
#include <QHBoxLayout>

#include <ambrosia/ambrosiawidget.h>
#include <utopia2/node.h>

#include <QVBoxLayout>
#include <QPushButton>

AmbrosiaPreview::AmbrosiaPreview(QWidget * parent, Qt::WindowFlags f)
    : AbstractPreview(parent, f)
{
    // AlignmentWidget
    this->_structureWidget = new AMBROSIA::AmbrosiaWidget();

    // Create and lay out widget
    QVBoxLayout * vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
//        vBoxLayout->addStretch(2);
    vBoxLayout->addWidget(this->_structureWidget);
//        vBoxLayout->addStretch(2);
}

AmbrosiaPreview::~AmbrosiaPreview()
{}

Utopia::Node * AmbrosiaPreview::type() const
{
    qDebug() << "AmbrosiaPreview::type()" << Utopia::Node::getNode("complex");
    return Utopia::Node::getNode("complex"); // FIXME
}

void AmbrosiaPreview::modelSet()
{
    if (this->model())
    {
        this->_structureWidget->show();
        this->_structureWidget->load(this->model());
        QSize hint = this->_structureWidget->sizeHint();
        this->_structureWidget->resize(hint);
    }
}
