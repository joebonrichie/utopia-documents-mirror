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

#include "reflection.h"

#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>

#include <QCursor>
#include <QWebView>

#include <QtDebug>




/////////////////////////////////////////////////////////////////////////////////////////
/// ReflectProcessor

ReflectProcessor::ReflectProcessor()
    : Papyro::AnnotationProcessor()
{}

ReflectProcessor::~ReflectProcessor()
{}

void ReflectProcessor::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    if (Papyro::PapyroWindow * window = Papyro::PapyroWindow::currentWindow()) {
        if (Papyro::PapyroTab * tab = window->currentTab()) {
            if (!annotations.empty()) {
                Spine::AnnotationHandle annotation = *annotations.begin();
                std::string url(annotation->getFirstProperty("property:webpageUrl"));
                std::string name(annotation->getFirstProperty("property:name"));
                std::string concept(annotation->getFirstProperty("concept"));
                if (!url.empty() && concept == "Reflection") {
                    QWebView * webView = new QWebView;
                    webView->page()->setNetworkAccessManager(tab->networkAccessManager());
                    webView->setWindowFlags(Qt::Tool);
                    webView->setWindowTitle(Papyro::qStringFromUnicode(name));
                    webView->setUrl(Papyro::qStringFromUnicode(url));
                    webView->setFixedSize(401, 243);
                    webView->show();
                    webView->move(globalPos);
                    webView->raise();
                }
            }
        }
    }
}

bool ReflectProcessor::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return annotation->getFirstProperty("concept") == "Reflection";
}

int ReflectProcessor::category() const
{
    return 10;
}

QString ReflectProcessor::title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return "Reflect";
}

int ReflectProcessor::weight() const
{
    return 10;
}
