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

#include <papyro/annotationresultitem_p.h>
#include <papyro/annotationresultitem.h>
#include <papyro/capabilities.h>
#include <papyro/utils.h>

#include <QMetaType>

namespace Papyro
{

    AnnotationResultItemPrivate::AnnotationResultItemPrivate(Spine::AnnotationHandle annotation, AnnotationResultItem * parent)
        : QObject(parent),
          annotation(annotation),
          contentState(ResultItem::Ungenerated)
    {
        // Order (somehow) the capabilities FIXME - this just puts them in the order they come back from Utopia
        int i = 0;
        foreach (boost::shared_ptr< SummaryCapability > renderer, annotation->capabilities< SummaryCapability >()) {
            QString key;
            key.setNum(++i);
            renderers[renderer] = key;
        }

        qRegisterMetaType< Papyro::ResultItem::State >("Papyro::ResultItem::State");
        connect(this, SIGNAL(contentChanged(const QString &)), parent, SIGNAL(contentChanged(const QString &)));
        connect(this, SIGNAL(stateChanged(Papyro::ResultItem::State)), parent, SIGNAL(stateChanged(Papyro::ResultItem::State)));
    }

    bool AnnotationResultItemPrivate::isFinished() const
    {
        QMapIterator< QString, QPair< bool, QStringList > > iter(content);
        while (iter.hasNext()) {
            iter.next();
            if (!iter.value().first) {
                return false;
            }
        }
        return true;
    }

    void AnnotationResultItemPrivate::receiveContent(QString key, QStringList content)
    {
        this->content[key] = qMakePair(true, content);

        if (isFinished()) {
            setState(ResultItem::Generated);
        }

        emit contentChanged(key);

    }

    void AnnotationResultItemPrivate::setState(ResultItem::State state)
    {
        if (contentState != state) {
            contentState = state;
            emit stateChanged(state);
        }
    }



    AnnotationResultItem::AnnotationResultItem(Spine::AnnotationHandle annotation, const QVariantMap & context)
        : ResultItem(context), d(new AnnotationResultItemPrivate(annotation, this))
    {}

    AnnotationResultItem::~AnnotationResultItem()
    {}

    Spine::AnnotationHandle AnnotationResultItem::annotation() const
    {
        return d->annotation;
    }

    QString AnnotationResultItem::title() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("property:name"));
    }

    QString AnnotationResultItem::description() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("property:description"));
    }

    QStringList AnnotationResultItem::content(const QString & key) const
    {
        return d->content.value(key).second;
    }

    QStringList AnnotationResultItem::contentKeys() const
    {
        return d->content.keys();
    }

    ResultItem::State AnnotationResultItem::contentState() const
    {
        return d->contentState;
    }

    QString AnnotationResultItem::authorUri() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("author"));
    }

    QString AnnotationResultItem::semanticType() const
    {
        return QString();
    }

    QString AnnotationResultItem::sourceDatabase() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("property:sourceDatabase"));
    }

    QString AnnotationResultItem::sourceDescription() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("property:sourceDescription"));
    }

    QUrl AnnotationResultItem::sourceIcon() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("property:sourceIcon"));
    }

    QUuid AnnotationResultItem::sourcePlugin() const
    {
        return QUuid(qStringFromUnicode(d->annotation->getFirstProperty("property:sourcePlugin")));
    }

    QString AnnotationResultItem::cssId() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("session:cssId"));
    }

    bool AnnotationResultItem::defaultness() const
    {
        if (!d->annotation->extents().empty() || !d->annotation->areas().empty() || !d->annotation->getFirstProperty("session:headless").empty()) {
            return true;
        } else {
            QString defaultness(qStringFromUnicode(d->annotation->getFirstProperty("session:default")).toLower());
            bool ok;
            int parsed = defaultness.toInt(&ok);
            return ok ? parsed != 0 : !defaultness.isEmpty();
        }
    }

    bool AnnotationResultItem::headless() const
    {
        return !d->annotation->getFirstProperty("session:headless").empty();
    }

    QString AnnotationResultItem::highlight() const
    {
        return qStringFromUnicode(d->annotation->getFirstProperty("session:highlight"));
    }

    int AnnotationResultItem::weight() const
    {
        QString defaultness(qStringFromUnicode(d->annotation->getFirstProperty("session:weight")));
        bool ok;
        int parsed = defaultness.toInt(&ok);
        return ok ? parsed : 0;
    }

    QString AnnotationResultItem::value(const QString & key) const
    {
        std::string qKey(unicodeFromQString(key));
        if (d->annotation->hasProperty(qKey)) {
            return qStringFromUnicode(d->annotation->getFirstProperty(qKey));
        }
        return QString();
    }

    QStringList AnnotationResultItem::values(const QString & key) const
    {
        QStringList all;
        std::string qKey(unicodeFromQString(key));
        if (d->annotation->hasProperty(qKey)) {
            foreach (const std::string & value, d->annotation->getProperty(qKey)) {
                all << qStringFromUnicode(value);
            }
        }
        return all;
    }

    void AnnotationResultItem::generateContent()
    {
        // Generate summaries
        if (d->contentState == ResultItem::Ungenerated) {
            if (!d->renderers.isEmpty()) {
                d->setState(ResultItem::Generating);
                // Execute renderers
                foreach (boost::shared_ptr< SummaryCapability > renderer, d->renderers.keys()) {
                    QString key = d->renderers.value(renderer);
                    renderer->generate(d->annotation, key, d, SLOT(receiveContent(QString, QStringList)));
                    d->content[key] = qMakePair< bool, QStringList >(false, QStringList());
                }
            } else {
                d->setState(ResultItem::Generated);
            }
        }
    }

    QList< Spine::CapabilityHandle > AnnotationResultItem::capabilities() const
    {
        QList< Spine::CapabilityHandle > list;
        foreach (Spine::CapabilityHandle capability, d->annotation->capabilities()) {
            list << capability;
        }
        return list;
    }
}
