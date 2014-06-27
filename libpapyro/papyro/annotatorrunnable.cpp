/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <papyro/annotatorrunnable.h>
#include <papyro/utils.h>

#include <QMutex>

namespace Papyro
{

    /// AnnotatorRunnablePrivate ////////////////////////////////////////////////////////

    class AnnotatorRunnablePrivate
    {
    public:
        AnnotatorRunnablePrivate()
            : mutex(QMutex::Recursive),
              runnable(true)
            {}

        boost::shared_ptr< Annotator > annotator;
        QString event;
        Spine::DocumentHandle document;
        QVariantMap kwargs;
        bool runnable;
        QString title;
        QMutex mutex;
    }; // class AnnotatorRunnablePrivate




    /// AnnotatorRunnable ///////////////////////////////////////////////////////////////

    AnnotatorRunnable::AnnotatorRunnable(boost::shared_ptr< Annotator > annotator,
                                         const QString & event,
                                         Spine::DocumentHandle document,
                                         const QVariantMap & kwargs)
        : QObject(), QRunnable(), d(new AnnotatorRunnablePrivate)
    {
        d->annotator = annotator;
        d->event = event;
        d->document = document;
        d->kwargs = kwargs;

        d->title = qStringFromUnicode(d->annotator->title());
    }

    const QString & AnnotatorRunnable::event() const
    {
        return d->event;
    }

    bool AnnotatorRunnable::isRunnable() const
    {
        QMutexLocker guard(&d->mutex);
        return d->runnable;
    }

    void AnnotatorRunnable::run()
    {
        if (isRunnable())
        {
            Q_EMIT started();
            d->annotator->handleEvent(d->event, d->document, d->kwargs);
            Q_EMIT finished(false);
        }
        else
        {
            Q_EMIT finished(true);
        }
    }

    void AnnotatorRunnable::skip()
    {
        QMutexLocker guard(&d->mutex);
        d->runnable = false;
    }

    const QString & AnnotatorRunnable::title() const
    {
        return d->title;
    }

} // namespace Papyro

