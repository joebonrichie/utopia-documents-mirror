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

#ifndef DOCUMENTFACTORY_H
#define DOCUMENTFACTORY_H

#include <papyro/config.h>
#include <spine/Annotation.h>
#include <spine/Document.h>
//#include <spine/TextAnnotationProxy.h>
#include <utopia2/extension.h>


#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QEventLoop>

namespace Utopia { class Node; }

namespace Papyro
{

    using Spine::Document;

    class LIBPAPYRO_API DocumentFactory
    {
    public:
        typedef DocumentFactory API;

        // Destructor
        virtual ~DocumentFactory() {};

        // Static method for loading a document into a Papyro Document
        static Spine::DocumentHandle load(const QString & filename = QString());
        static Spine::DocumentHandle load(Utopia::Node * model = 0);

        // Produce a document
        virtual Spine::DocumentHandle create(const QByteArray & bytes) = 0;
        virtual Spine::DocumentHandle create(const QString & filename);
        // Check to see if this factory is capable of producing a document
        virtual bool isCapable(const QString & filename) = 0;

    };

}

UTOPIA_DECLARE_EXTENSION_CLASS(LIBPAPYRO, Papyro::DocumentFactory)

#endif // DOCUMENTFACTORY_H
