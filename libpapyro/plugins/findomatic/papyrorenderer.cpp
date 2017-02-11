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
#include <papyro/documentview.h>
#include <papyro/pageview.h>


#include "papyrorenderer.h"
#include <QImage>

#define TOP_PADDING 2
//#define LEFT_PADDING 290
#define TEXT_GAP 10

PapyroRenderer::PapyroRenderer()
{}

PapyroRenderer::~PapyroRenderer()
{}

QImage PapyroRenderer::iconSmall(const QModelIndex& index)
{
    return QImage(":/images/default_small.png");
}

QImage PapyroRenderer::iconLarge(const QModelIndex& index)
{
    Utopia::Node* node = index.data(Qt::UserRole).value< Utopia::Node* >();
//      QString name= node->attributes.get("DocumentID").toString();

    // FIXME
    // Create / access cached / whatever - thumbnail image for this model.

/*
  bool found= false;
  PAPYRO::Document *document;
  for (int i=0; i<PAPYRO::Document::documents.size() && !found; ++i)
  {
  document= PAPYRO::Document::documents[i];
  if (document->title() == name)
  {
  found= true;
  }
  }

  if (found)
  {
  if (iconPreviewMap.contains(index))
  {
  return iconPreviewMap[index];
  }
  else
  {
  QImage previewPage= document->page(0)->image(0.3, QColor(Qt::white))->toImage();
//                      QImage previewDecorated(previewPage.size() + QSize(5,5), QImage::Format_RGB32);
//                      previewDecorated.fill(QColor(Qt::white).rgb());
//                      QPainter painter(&previewDecorated);
//                      PageView::compositePage(painter, previewPage, 0.3);
iconPreviewMap[index]= previewPage.scaledToHeight(124, Qt::SmoothTransformation);
return iconPreviewMap[index];
}
}
else */
    {
        return QImage(":/images/default_large.png");
    }
}

QMap< QString, QVariant > PapyroRenderer::attributes(const QModelIndex & index) const
{
    Utopia::Node * node = index.data(Qt::UserRole).value< Utopia::Node * >();

    QMap< QString, QVariant > map;
    map.insert("Title", node->attributes.get("papyro:title", "Untitled").toString());
    QString fullPath = node->attributes.get("papyro:pdfFile", "No PDF File Attached").toString();
    QString filename = fullPath.split("/").last();
    map.insert("File", filename);
    return map;

    // return QMap< QString, QVariant >();
}

QMap< QString, QString > PapyroRenderer::primaryAttributes(const QModelIndex & index) const
{
    Utopia::Node * p_hasAuthor = Utopia::UtopiaDomain.term("hasAuthor");
    Utopia::Node * node = index.data(Qt::UserRole).value< Utopia::Node * >();

    QMap< QString, QString > map;
    map["primaryIdentifier"] = node->attributes.get("papyro:title", "Untitled").toString();

    QString authors = "Unknown Authors";
    int authorCount = node->relations(p_hasAuthor).size();
    Utopia::Node::relation::iterator i = node->relations(p_hasAuthor).begin();
    Utopia::Node::relation::iterator end = node->relations(p_hasAuthor).end();
    for (int author = 0; i != end; ++i, ++author)
    {
        //QString authorName = (*i)->attributes.get("papyro:forename", "").toString() + " " + (*i)->attributes.get("papyro:surname", "").toString();
        QString authorName = (*i)->attributes.get("papyro:surname", "").toString();
        if (author == 0)
        {
            authors = authorName;
        }
        else
        {
            if (author == authorCount - 1 )
            {
                authors += " and " + authorName;
            }
            else
            {
                authors += ", " + authorName;
            }
        }
    }

    map["secondaryIdentifier"] = authors;
    map["source"] = node->attributes.get("source", "").toString();
    map["date"] = node->attributes.get("date", QDate(2000, 01, 01)).toString();

    return map;
}

Utopia::Node * PapyroRenderer::type() const
{
    return Utopia::UtopiaDomain.term("Document");
}
