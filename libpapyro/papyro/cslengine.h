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

#ifndef PAPYRO_CSLENGINE_H
#define PAPYRO_CSLENGINE_H

#include <QObject>
#include <QString>
#include <QVariantMap>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <boost/shared_ptr.hpp>
#endif

namespace Papyro
{

    //
    //  For the format of CSL metadata see:
    //    http://citationstyles.org/downloads/specification.html#appendix-iv-variables
    //


    class CSLEnginePrivate;
    class CSLEngine : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString defaultStyle
                   READ defaultStyle
                   WRITE setDefaultStyle)

    public:
        CSLEngine(QObject * parent = 0);
        ~CSLEngine();

        QVariantMap availableLocales() const;
        QVariantMap availableStyles() const;
        const QString & defaultStyle() const;
        QString format(const QVariantMap & metadata, const QString & style = QString());

        void setDefaultStyle(const QString & defaultStyle);

        static boost::shared_ptr< CSLEngine > instance();

    protected:
        CSLEnginePrivate * d;
    }; // class CSLEngine

} // namespace Papyro

#endif // PAPYRO_CSLENGINE_H
