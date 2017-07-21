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

#ifndef PAPYRO_RESULTITEM_H
#define PAPYRO_RESULTITEM_H

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <spine/Capability.h>
#endif

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUuid>
#include <QVariantMap>

namespace Papyro
{

    class ResultItemPrivate;
    class ResultItem : public QObject
    {
        Q_OBJECT

    public:
        enum State { Ungenerated, Generating, Generated };
        Q_ENUMS(State);

        ResultItem(const QVariantMap & context = QVariantMap());
        virtual ~ResultItem();

        virtual QString title() const = 0;
        virtual QString description() const = 0;

        virtual QString authorUri() const = 0;
        virtual QString semanticType() const = 0;
        virtual QString sourceDatabase() const = 0;
        virtual QString sourceDescription() const = 0;
        virtual QUrl sourceIcon() const = 0;
        virtual QUuid sourcePlugin() const = 0;

        virtual QString cssId() const;
        virtual bool defaultness() const;
        virtual bool headless() const;
        virtual QString highlight() const;
        virtual int weight() const;

        virtual QStringList values(const QString & key) const = 0;
        virtual QString value(const QString & key) const;

        virtual QVariantMap context() const;

        virtual State contentState() const = 0;
        virtual QStringList content(const QString & key) const = 0;
        virtual QStringList contentKeys() const = 0;

        virtual QList< Spine::CapabilityHandle > capabilities() const;

        template< class CapabilityClass >
        QList< boost::shared_ptr< CapabilityClass > > capabilities() const
        {
            QList< boost::shared_ptr< CapabilityClass > > matches;
            foreach (Spine::CapabilityHandle capability, capabilities()) {
                boost::shared_ptr< CapabilityClass > casted_capability = boost::dynamic_pointer_cast< CapabilityClass, Spine::Capability >(capability);
                if (casted_capability) {
                    matches << casted_capability;
                }
            }
            return matches;
        }

    public slots:
        virtual void generateContent();

    signals:
        void contentChanged(const QString & key);
        void stateChanged(Papyro::ResultItem::State state);

    protected:
        ResultItemPrivate * d;
    };

}

#endif // PAPYRO_RESULTITEM_H
