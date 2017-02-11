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

#ifndef PAPYRO_DISPATCHER_P_H
#define PAPYRO_DISPATCHER_P_H

#include <papyro/config.h>

#if !defined(Q_MOC_RUN) || QT_VERSION >= 0x050000
#  include <papyro/annotator.h>
#  include <spine/Annotation.h>
#  include <spine/Document.h>
#  include <boost/shared_ptr.hpp>
#endif

#include <QMutex>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QThreadPool>

namespace Papyro
{

    class Dispatcher;
    class DispatcherPrivate;

    class LIBPAPYRO_API DispatchEngine : public QThread
    {
        Q_OBJECT

    public:
        DispatchEngine(Dispatcher * parent,
                       DispatcherPrivate * dispatcherPrivate,
                       Spine::DocumentHandle document,
                       const QStringList & terms);
        ~DispatchEngine();

        void cancel();
        void run();
        void detach();
        bool detached() const;

    signals:
        void annotationFound(Spine::AnnotationHandle annotation);
        void cancellationRequested();
        void finished();

    private:
        DispatcherPrivate * d;

        mutable QMutex mutex;
        QQueue< QString > queue;
        QThreadPool threadPool;
        bool cancelled;

        Spine::DocumentHandle document;

        Dispatcher * dispatcher();

    }; /* class DispatcherEnginex */

    class LookupRunnable : public QObject, public QRunnable
    {
        Q_OBJECT

    public:
        LookupRunnable(DispatcherPrivate * d, DispatchEngine * engine, Spine::DocumentHandle document, boost::shared_ptr< Annotator > annotator, const QStringList & terms);
        ~LookupRunnable();

        void run();

    signals:
        void annotationFound(Spine::AnnotationHandle annotation);

    protected:
        DispatcherPrivate * d;
        DispatchEngine * engine;
        Spine::DocumentHandle document;
        boost::shared_ptr< Annotator > annotator;
        QStringList terms;
    };

} // namespace Papyro

#endif // PAPYRO_DISPATCHER_P_H
