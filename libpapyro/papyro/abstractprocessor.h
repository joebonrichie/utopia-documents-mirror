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

#ifndef PAPYRO_ABSTRACTPROCESSOR_H
#define PAPYRO_ABSTRACTPROCESSOR_H

#include <papyro/config.h>
#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Cursor.h>
#  include <spine/Document.h>
#endif

#include <QIcon>
#include <QString>

namespace Papyro
{

    class LIBPAPYRO_API AbstractProcessor
    {
    public:
        virtual ~AbstractProcessor() {}

        virtual int category() const { return 0; }
        virtual QIcon icon() const { return QIcon(); }
        virtual QString title() const { return QString(); }
        virtual int weight() const { return 0; }

        static QIcon generateFromMonoPixmap(const QPixmap & pixmap);

    }; // class AbstractProcessor

}

#endif // PAPYRO_ABSTRACTPROCESSOR_H
