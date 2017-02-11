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

#ifndef PAPYRO_OVERLAYRENDERERMAPPER_H
#define PAPYRO_OVERLAYRENDERERMAPPER_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/Document.h>
#  include <utopia2/extension.h>
#endif

#include <QString>
#include <QVariantMap>

namespace Papyro
{

    class OverlayRendererMapper
    {
    public:
        typedef OverlayRendererMapper API;

        virtual ~OverlayRendererMapper();

        virtual QString mapToId(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const = 0;
        virtual int weight() const { return 0; }
    }; // class OverlayRendererMapper




    class PropertyOverlayRendererMapper : public OverlayRendererMapper
    {
    public:
        PropertyOverlayRendererMapper(const QVariantMap & criteria = QVariantMap(), const QString & id = QString());
        QVariantMap criteria() const;
        QString id() const;
        QString mapToId(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const;
    protected:
        void setCriteria(const QVariantMap & criteria);
        void setId(const QString & id);
    private:
        QVariantMap _criteria;
        QString _id;
    }; // class PropertyOverlayRendererMapper




    class ConceptOverlayRendererMapper : public PropertyOverlayRendererMapper
    {
    public:
        ConceptOverlayRendererMapper(const QString & concept, const QString & id);
    }; // class ConceptOverlayRendererMapper

} // namespace Papyro

UTOPIA_DECLARE_EXTENSION_CLASS(LIBPAPYRO, Papyro::OverlayRendererMapper)

#endif // PAPYRO_OVERLAYRENDERERMAPPER_H
