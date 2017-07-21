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
#include <papyro/documentproxy_p.h>

namespace Papyro
{

    namespace
    {
        void slot_annotationsChanged(void * userdef, const std::string &name, Spine::AnnotationSet annotations, bool added)
        {
            DocumentProxyPrivate * proxy = static_cast< DocumentProxyPrivate * >(userdef);
            proxy->onAnnotationsChanged(name, annotations, added);
        }

        void slot_areaSelectionChanged(void * userdef, const std::string &name, Spine::AreaSet areas, bool added)
        {
            DocumentProxyPrivate * proxy = static_cast< DocumentProxyPrivate * >(userdef);
            proxy->onAreaSelectionChanged(name, areas, added);
        }

        void slot_textSelectionChanged(void * userdef, const std::string &name, Spine::TextExtentSet extents, bool added)
        {
            DocumentProxyPrivate * proxy = static_cast< DocumentProxyPrivate * >(userdef);
            proxy->onTextSelectionChanged(name, extents, added);
        }
    } // anonymous namespace




    DocumentProxyPrivate::DocumentProxyPrivate(DocumentProxy * proxy)
        : QObject(proxy), proxy(proxy)
    {
        // Register metatypes
        qRegisterMetaType< Spine::AnnotationSet >("Spine::AnnotationSet");
        qRegisterMetaType< Spine::TextExtentSet >("Spine::TextExtentSet");
        qRegisterMetaType< Spine::AreaSet >("Spine::AreaSet");
        qRegisterMetaType< std::string >("std::string");

        connect(this, SIGNAL(annotationsChanged(std::string, Spine::AnnotationSet, bool)),
                proxy, SIGNAL(annotationsChanged(std::string, Spine::AnnotationSet, bool)));
        connect(this, SIGNAL(areaSelectionChanged(std::string, Spine::AreaSet, bool)),
                proxy, SIGNAL(areaSelectionChanged(std::string, Spine::AreaSet, bool)));
        connect(this, SIGNAL(textSelectionChanged(std::string, Spine::TextExtentSet, bool)),
                proxy, SIGNAL(textSelectionChanged(std::string, Spine::TextExtentSet, bool)));
    }

    void DocumentProxyPrivate::onAnnotationsChanged(const std::string & name, const Spine::AnnotationSet & annotations, bool added)
    {
        if (document) {
            emit annotationsChanged(name, annotations, added);
        }
    }

    void DocumentProxyPrivate::onAreaSelectionChanged(const std::string & name, const Spine::AreaSet & areas, bool added)
    {
        if (document) {
            emit areaSelectionChanged(name, areas, added);
        }
    }

    void DocumentProxyPrivate::onTextSelectionChanged(const std::string & name, const Spine::TextExtentSet & extents, bool added)
    {
        if (document) {
            emit textSelectionChanged(name, extents, added);
        }
    }




    DocumentProxy::DocumentProxy(QObject * parent)
        : QObject(parent), d(new DocumentProxyPrivate(this))
    {}

    DocumentProxy::~DocumentProxy()
    {
        // Reset document before closing
        setDocument(Spine::DocumentHandle());
    }

    Spine::DocumentHandle DocumentProxy::document() const
    {
        return d->document;
    }

    void DocumentProxy::setDocument(Spine::DocumentHandle document)
    {
        if (d->document) {
            d->document->disconnectAnyAnnotationsChanged(slot_annotationsChanged, d);
            d->document->disconnectAnyAreaSelectionChanged(slot_areaSelectionChanged, d);
            d->document->disconnectAnyTextSelectionChanged(slot_textSelectionChanged, d);
        }
        d->document = document;
        if (d->document) {
            d->document->connectAnyAnnotationsChanged(slot_annotationsChanged, d);
            d->document->connectAnyAreaSelectionChanged(slot_areaSelectionChanged, d);
            d->document->connectAnyTextSelectionChanged(slot_textSelectionChanged, d);
        }
        emit documentChanged();
    }

} // namespace Papyro
