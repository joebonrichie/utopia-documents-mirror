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

#include "citations.h"

#include <papyro/citationpopup.h>
#include <papyro/documentview.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>

#include <QCursor>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <QtDebug>




/////////////////////////////////////////////////////////////////////////////////////////
/// CitationActivator

CitationActivator::CitationActivator()
    : Papyro::AnnotationProcessor()
{}

CitationActivator::~CitationActivator()
{}

void CitationActivator::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    if (!annotations.empty()) {
        Papyro::CitationPopup * popup = new Papyro::CitationPopup;
        std::set< std::string > ids;
        foreach (Spine::AnnotationHandle citation, annotations) {
            std::string bibid = citation->getFirstProperty("property:id");
            if (ids.find(bibid) == ids.end()) {
                if (bibid.size() > 0) {
                    popup->addCitation(citation);
                }
                ids.insert(bibid);
            }
        }
        popup->setAttribute(Qt::WA_DeleteOnClose, true);
        QObject::connect(popup, SIGNAL(requestUrl(const QUrl &, const QString &)), Papyro::PapyroWindow::currentWindow(), SLOT(requestUrl(const QUrl &, const QString &)));
        popup->adjustSize();
        popup->show(globalPos);
    }
}

QList< Spine::AnnotationSet > CitationActivator::canActivate(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    Spine::AnnotationSet valid;
    foreach (Spine::AnnotationHandle annotation, annotations) {
        if (canActivate(document, annotation)) {
            valid.insert(annotation);
        }
    }
    if (valid.empty()) {
        return QList< Spine::AnnotationSet >();
    } else {
        return QList< Spine::AnnotationSet >() << valid;
    }
}

bool CitationActivator::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    std::string concept(annotation->getFirstProperty("concept"));
    return (concept == "ForwardCitation" || concept == "Citation");
}

int CitationActivator::category() const
{
    return 0;
}

QIcon CitationActivator::icon(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    return QIcon();
}

QString CitationActivator::title(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    return QString("View citation") + (annotations.size() == 1 ? "" : "s") + "...";
}
