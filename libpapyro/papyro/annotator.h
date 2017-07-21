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

#ifndef PAPYRO_ANNOTATOR_H
#define PAPYRO_ANNOTATOR_H

#include <papyro/config.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Annotation.h>
#  include <spine/Document.h>
#endif
#include <string>
#include <utopia2/extension.h>
#include <utopia2/configurable.h>
#include <utopia2/busagent.h>

#include <QStringList>
#include <QVariantMap>
#include <set>

namespace Papyro
{

    class LIBPAPYRO_API Annotator : public virtual Utopia::Configurable, public Utopia::BusAgent
    {
    public:
        typedef Annotator API;

        virtual void cancel() {}

        // Event types: init, load, ready, filter, activate, marshal, persist, lookup
        // Event timings: before, on, after
        // Event name: timing:type (on:filter, before:marshal, etc.)
        virtual bool canHandleEvent(const QString & event) { return false; }
        virtual QStringList handleableEvents() { return QStringList(); }
        virtual bool handleEvent(const QString & event, Spine::DocumentHandle document, const QVariantMap & kwargs = QVariantMap()) { return false; }

        /** Lookup framework **/

        virtual std::set< Spine::AnnotationHandle > lookup(Spine::DocumentHandle document, const std::string & phrase, const QVariantMap & kwargs = QVariantMap())
        { return std::set< Spine::AnnotationHandle >(); }

        virtual std::string title() = 0;
        //virtual std::string uuid() = 0;

        std::string errorString() const { return _errorString; }
        void setErrorString(const std::string & errorString) { _errorString = errorString; }

    private:
        std::string _errorString;
    }; /* class Annotator */

} // namespace Papyro

UTOPIA_DECLARE_EXTENSION_CLASS(LIBPAPYRO, Papyro::Annotator)

#endif // PAPYRO_ANNOTATOR_H
