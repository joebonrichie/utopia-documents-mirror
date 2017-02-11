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

#ifndef PAPYRO_CITATIONPOPUP_P_H
#define PAPYRO_CITATIONPOPUP_P_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#endif
#include <utopia2/qt/spinner.h>
#include <papyro/citation.h>

#include <QMenu>
#include <QObject>
#include <QSignalMapper>
#include <QWidget>

class QLabel;
class QHBoxLayout;
class QStackedLayout;
class QVBoxLayout;

namespace Papyro
{

    class CitationPanel : public QWidget
    {
        Q_OBJECT

        public:
            CitationPanel(Spine::AnnotationHandle citation);

            Spine::AnnotationHandle citation;

        signals:
            void requestUrl(const QUrl & url, const QString & target);

        public slots:
            void addLink(QString, QString);
            void addPdf(QString, QString);
            void onLinkClicked(const QString & url);
            void onResolverRunnableCompleted(Athenaeum::CitationHandle citation);

        protected:
            bool eventFilter(QObject * obj, QEvent * event);

            QHBoxLayout * _layout;
            QLabel * citationLabel;
            QLabel * pdfLabel;
            QLabel * moreLabel;
            Utopia::Spinner * spinner;
            QStackedLayout * moreLayout;
            QStackedLayout * pdfLayout;
            QMenu menu;
            QSignalMapper signalMapper;
            int completed;
    };

    class CitationPopupPrivate : public QObject
    {
        Q_OBJECT

        public:
            CitationPopupPrivate(QObject * parent)
                : QObject(parent), layout(0), numerical(true)
            {}

            QVBoxLayout * layout;
            bool numerical;
    };

}

#endif // PAPYRO_CITATIONPOPUP_P_H
