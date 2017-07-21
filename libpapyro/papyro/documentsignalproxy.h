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

#ifndef documentProxy_H
#define documentProxy_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/Area.h>
#  include <spine/Document.h>
#  include <spine/TextSelection.h>
#endif

#include <string>
#include <QObject>

namespace Papyro
{

    class documentProxy : public QObject
    {
        Q_OBJECT

    public:
        documentProxy(QObject * parent = 0);
        documentProxy(Spine::DocumentHandle document, QObject * parent = 0);
        ~documentProxy();

        Spine::DocumentHandle document() const;
        void setDocument(Spine::DocumentHandle document);

    public:
        void onAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void onAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added);
        void onTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added);

    signals:
        void documentChanged();

        void annotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added);
        void areaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added);
        void textSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added);

        void deferAnnotationsChanged(std::string name, Spine::AnnotationSet annotations, bool added);
        void deferAreaSelectionChanged(std::string name, Spine::AreaSet areas, bool added);
        void deferTextSelectionChanged(std::string name, Spine::TextExtentSet extents, bool added);

    protected slots:
        void onDeferredAnnotationsChanged(std::string name, Spine::AnnotationSet annotations, bool added);
        void onDeferredAreaSelectionChanged(std::string name, Spine::AreaSet areas, bool added);
        void onDeferredTextSelectionChanged(std::string name, Spine::TextExtentSet extents, bool added);

    protected:
        Spine::DocumentHandle _document;
    };

}

#endif // documentProxy_H
