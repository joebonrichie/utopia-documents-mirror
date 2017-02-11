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

#include "wiley.h"

#include <papyro/documentview.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>
#include <utopia2/qt/bubble.h>

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
/// WileyActivator

WileyActivator::WileyActivator()
    : Papyro::AnnotationProcessor()
{}

WileyActivator::~WileyActivator()
{}

void WileyActivator::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    static const QString tpl = "<strong>%1</strong> &mdash; <span>%2</span>";
    if (!annotations.empty()) {
        Utopia::BubbleWidget * popup = new Utopia::BubbleWidget(0, Qt::Popup);
        popup->setStyleSheet("Utopia--BubbleWidget { qproperty-bubbleBackground: #f6f6ff; qproperty-bubbleBorder: #667; color: black; }");
        popup->setPopup(true);
        popup->setFixedWidth(400);
        QVBoxLayout * layout = new QVBoxLayout(popup);
        layout->setSpacing(8);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSizeConstraint(QLayout::SetFixedSize);
        std::set< std::string > ids;
        foreach (Spine::AnnotationHandle citation, annotations) {
            QString term = Papyro::qStringFromUnicode(citation->getFirstProperty("property:term"));
            QString definition = Papyro::qStringFromUnicode(citation->getFirstProperty("property:definition"));
            if (!definition.endsWith(".")) {
                definition += ".";
            }
            QLabel * label = new QLabel;
            label->setWordWrap(true);
            label->setFixedWidth(400);
            label->setText(tpl.arg(term).arg(definition));
            label->setStyleSheet("color: black;");
            label->setFixedHeight(label->heightForWidth(label->width()));
            layout->addWidget(label);
        }
        popup->setAttribute(Qt::WA_DeleteOnClose, true);
        popup->adjustSize();
        popup->show(globalPos);
    }
}

QList< Spine::AnnotationSet > WileyActivator::canActivate(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
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

bool WileyActivator::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    std::string concept(annotation->getFirstProperty("concept"));
    return (concept == "WileyDefinition");
}

int WileyActivator::category() const
{
    return 0;
}

QIcon WileyActivator::icon(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    return QIcon();
}

QString WileyActivator::title(Spine::DocumentHandle document, Spine::AnnotationSet annotations) const
{
    return QString("View definition") + (annotations.size() == 1 ? "" : "s") + "...";
}
