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

#include <papyro/overlayrenderermapper.h>
#include <papyro/utils.h>

#include <QDebug>

namespace Papyro
{

    OverlayRendererMapper::~OverlayRendererMapper()
    {}




    PropertyOverlayRendererMapper::PropertyOverlayRendererMapper(const QVariantMap & criteria, const QString & id)
        : _criteria(criteria), _id(id)
    {}

    QVariantMap PropertyOverlayRendererMapper::criteria() const
    {
        return _criteria;
    }

    QString PropertyOverlayRendererMapper::id() const
    {
        return _id;
    }

    QString PropertyOverlayRendererMapper::mapToId(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
    {
        QMapIterator< QString, QVariant > iter(_criteria);
        while (iter.hasNext()) {
            iter.next();
            // Empty value represents a check for a property's existence, regardless of value
            if (iter.value().isNull() && !annotation->hasProperty(Papyro::unicodeFromQString(iter.key()))) {
                return QString();
            } else if (!annotation->hasProperty(Papyro::unicodeFromQString(iter.key()), Papyro::unicodeFromQString(iter.value().toString()))) {
                return QString();
            }
        }
        return _id;
    }

    void PropertyOverlayRendererMapper::setCriteria(const QVariantMap & criteria)
    {
        _criteria = criteria;
    }

    void PropertyOverlayRendererMapper::setId(const QString & id)
    {
        _id = id;
    }




    ConceptOverlayRendererMapper::ConceptOverlayRendererMapper(const QString & concept, const QString & id)
        : PropertyOverlayRendererMapper()
    {
        QVariantMap crit;
        crit["concept"] = concept;
        setCriteria(crit);
        setId(id);
    }

} // namespace Papyro

UTOPIA_DEFINE_EXTENSION_CLASS(Papyro::OverlayRendererMapper)
