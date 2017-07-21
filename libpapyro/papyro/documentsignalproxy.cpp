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

#include <papyro/documentproxy.h>
#include <spine/Document.h>
#include <QMetaType>

namespace Papyro
{

    // Signal callbacks

    namespace
    {

        void slot_annotationsChanged(void * userdef, const std::string &name, Spine::AnnotationSet annotations, bool added)
        {
            documentProxy * proxy = static_cast< documentProxy * >(userdef);
            proxy->onAnnotationsChanged(name, annotations, added);
        }

        void slot_areaSelectionChanged(void * userdef, const std::string &name, Spine::AreaSet areas, bool added)
        {
            documentProxy * proxy = static_cast< documentProxy * >(userdef);
            proxy->onAreaSelectionChanged(name, areas, added);
        }

        void slot_textSelectionChanged(void * userdef, const std::string &name, Spine::TextExtentSet extents, bool added)
        {
            documentProxy * proxy = static_cast< documentProxy * >(userdef);
            proxy->onTextSelectionChanged(name, extents, added);
        }

    }


    documentProxy::documentProxy(QObject * parent)
        : QObject(parent)
    {
        // Deferred signals
        qRegisterMetaType< Spine::AnnotationSet >("Spine::AnnotationSet");
        qRegisterMetaType< Spine::TextExtentSet >("Spine::TextExtentSet");
        qRegisterMetaType< Spine::AreaSet >("Spine::AreaSet");
        qRegisterMetaType< std::string >("std::string");
        connect(this, SIGNAL(deferAnnotationsChanged(std::string,Spine::AnnotationSet,bool)),
                this, SLOT(onDeferredAnnotationsChanged(std::string,Spine::AnnotationSet,bool)));
        connect(this, SIGNAL(deferAreaSelectionChanged(std::string,Spine::AreaSet,bool)),
                this, SLOT(onDeferredAreaSelectionChanged(std::string,Spine::AreaSet,bool)));
        connect(this, SIGNAL(deferTextSelectionChanged(std::string,Spine::TextExtentSet,bool)),
                this, SLOT(onDeferredTextSelectionChanged(std::string,Spine::TextExtentSet,bool)));
    }

    documentProxy::documentProxy(Spine::DocumentHandle document, QObject * parent)
        : QObject(parent), _document(document)
    {
        // Deferred signals
        qRegisterMetaType< Spine::AnnotationSet >("Spine::AnnotationSet");
        qRegisterMetaType< Spine::TextExtentSet >("Spine::TextExtentSet");
        qRegisterMetaType< Spine::AreaSet >("Spine::AreaSet");
        qRegisterMetaType< std::string >("std::string");
        connect(this, SIGNAL(deferAnnotationsChanged(std::string,Spine::AnnotationSet,bool)),
                this, SLOT(onDeferredAnnotationsChanged(std::string,Spine::AnnotationSet,bool)));
        connect(this, SIGNAL(deferAreaSelectionChanged(std::string,Spine::AreaSet,bool)),
                this, SLOT(onDeferredAreaSelectionChanged(std::string,Spine::AreaSet,bool)));
        connect(this, SIGNAL(deferTextSelectionChanged(std::string,Spine::TextExtentSet,bool)),
                this, SLOT(onDeferredTextSelectionChanged(std::string,Spine::TextExtentSet,bool)));

        // Connect to Document
        setDocument(document);
    }

    documentProxy::~documentProxy()
    {
        setDocument(Spine::DocumentHandle());
    }

    Spine::DocumentHandle documentProxy::document() const
    {
        return _document;
    }

    void documentProxy::onDeferredAnnotationsChanged(std::string name, Spine::AnnotationSet annotations, bool added)
    {
        if (_document) {
            Q_EMIT annotationsChanged(name, annotations, added);
        }
    }

    void documentProxy::onDeferredAreaSelectionChanged(std::string name, Spine::AreaSet areas, bool added)
    {
        if (_document) {
            Q_EMIT areaSelectionChanged(name, areas, added);
        }
    }

    void documentProxy::onDeferredTextSelectionChanged(std::string name, Spine::TextExtentSet extents, bool added)
    {
        if (_document) {
            Q_EMIT textSelectionChanged(name, extents, added);
        }
    }

    void documentProxy::onAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added)
    {
        if (_document) {
            Q_EMIT deferAnnotationsChanged(name, annotations, added);
        }
    }

    void documentProxy::onAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added)
    {
        if (_document) {
            Q_EMIT deferAreaSelectionChanged(name, areas, added);
        }
    }

    void documentProxy::onTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added)
    {
        if (_document) {
            Q_EMIT deferTextSelectionChanged(name, extents, added);
        }
    }

    void documentProxy::setDocument(Spine::DocumentHandle document_)
    {
        if (_document) {
            _document->disconnectAnyAnnotationsChanged(slot_annotationsChanged, this);
            _document->disconnectAnyAreaSelectionChanged(slot_areaSelectionChanged, this);
            _document->disconnectAnyTextSelectionChanged(slot_textSelectionChanged, this);
        }
        _document = document_;
        if (_document) {
            _document->connectAnyAnnotationsChanged(slot_annotationsChanged, this);
            _document->connectAnyAreaSelectionChanged(slot_areaSelectionChanged, this);
            _document->connectAnyTextSelectionChanged(slot_textSelectionChanged, this);
        }
        emit documentChanged();
    }

}
