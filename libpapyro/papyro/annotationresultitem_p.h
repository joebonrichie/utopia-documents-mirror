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

#ifndef PAPYRO_ANNOTATIONRESULTITEM_P_H
#define PAPYRO_ANNOTATIONRESULTITEM_P_H

#include <papyro/resultitem.h>
#include <spine/Annotation.h>

#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>

namespace Papyro
{

    class AnnotationResultItem;
    class SummaryCapability;

    class AnnotationResultItemPrivate : public QObject
    {
        Q_OBJECT

    public:
        AnnotationResultItemPrivate(Spine::AnnotationHandle annotation, AnnotationResultItem * parent = 0);

        Spine::AnnotationHandle annotation;
        QMap< QString, QPair< bool, QStringList > > content;
        ResultItem::State contentState;
        QMap< boost::shared_ptr< SummaryCapability >, QString > renderers;

        void setState(ResultItem::State state);
        bool isFinished() const;

    signals:
        void contentChanged(const QString & key);
        void stateChanged(Papyro::ResultItem::State state);

    public slots:
        void receiveContent(QString key, QStringList content);
    };

}

#endif // PAPYRO_ANNOTATIONRESULTITEM_P_H
