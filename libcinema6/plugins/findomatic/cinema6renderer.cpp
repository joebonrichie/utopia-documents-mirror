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

#include <iostream>

#include <utopia2/node.h>
#include <utopia2/ontology.h>

#include "cinema6renderer.h"
#include <QImage>

#define TOP_PADDING 2
//#define LEFT_PADDING 290
#define TEXT_GAP 10

CinemaRenderer::CinemaRenderer()
{}

CinemaRenderer::~CinemaRenderer()
{}

QImage CinemaRenderer::iconSmall(const QModelIndex& index)
{
    return QImage(":/images/default_small.png");
}

QImage CinemaRenderer::iconLarge(const QModelIndex& index)
{
    //Utopia::Node* node = index.data(Qt::UserRole).value< Utopia::Node* >();
    return QImage(":/images/default_large.png");
}

QMap< QString, QVariant > CinemaRenderer::attributes(const QModelIndex& index) const
{
    //Utopia::Node* node = index.data(Qt::UserRole).value< Utopia::Node* >();
    QMap< QString, QVariant > map;
    map.insert("Title", "Title from sequence object");
    map.insert("Organism", "Organism from sequence object");

    return map;
}

QMap< QString, QString > CinemaRenderer::primaryAttributes(const QModelIndex& index) const
{
    QMap< QString, QString > map;

    map["primaryIdentifier"] = "Unknown A";
    map["secondaryIdentifier"] = "Unknown B";
    map["source"] = "Unknown source";
    map["date"] = "Unknown Date";

    return map;
}

Utopia::Node * CinemaRenderer::type() const
{
    return Utopia::UtopiaDomain.term("Sequence");
}
