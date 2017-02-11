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

#ifndef PAPYRO_ANNOTATORRUNNABLEPOOL_H
#define PAPYRO_ANNOTATORRUNNABLEPOOL_H

#include <QList>
#include <QObject>
#include <QThreadPool>

namespace Papyro
{

    class AnnotatorRunnable;




    /// AnnotatorRunnablePool ///////////////////////////////////////////////////////////

    class AnnotatorRunnablePoolPrivate;
    class AnnotatorRunnablePool : public QObject
    {
        Q_OBJECT

    public:
        AnnotatorRunnablePool(QObject * parent = 0);
        virtual ~AnnotatorRunnablePool();

        bool isActive();
        void skip();
        void start(QList< AnnotatorRunnable * > runnables, int priority = 0);
        void start(AnnotatorRunnable * runnable, int priority = 0);
        void sync();
        void sync(const QObject * receiver, const char * method, Qt::ConnectionType type = Qt::AutoConnection);
        void waitForDone();

    Q_SIGNALS:
        void started();
        void synced();
        void finished();

    protected Q_SLOTS:
        void onStarted();
        void onFinished();

    protected:
        void _start(AnnotatorRunnable * runnable, int priority);

    private:
        AnnotatorRunnablePoolPrivate * d;
    };

} // namespace Papyro

#endif // PAPYRO_ANNOTATORRUNNABLEPOOL_H
