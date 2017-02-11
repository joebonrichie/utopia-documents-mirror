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

#ifndef Utopia_SERVICE_INVOCATION_SET_H
#define Utopia_SERVICE_INVOCATION_SET_H

#include <utopia2/config.h>
#include <utopia2/invocation.h>
#include <utopia2/threads.h>
#include <list>
#include <set>
#include <vector>

#include <fstream>

#include <QObject>

namespace boost { class thread; }

namespace Utopia
{
    class Plugin;

    /**
     *  \class InvocationSet
     *
     *  Encapsulates a service Invocation set.
     */

    class LIBUTOPIA_API InvocationSet : public QObject
    {
        Q_OBJECT

    public:
        // Convenience
        typedef Invocation::ErrorCode ErrorCode;

        // Status struct
        typedef struct
        {
            std::set< Invocation * > running;
            std::set< Invocation * > completed;
            std::set< Invocation * > failed;
            double progress;
        } StatusInfo;

        // Constructor
        InvocationSet(size_t threadCount_ = 10);

        // Destructor
        ~InvocationSet();

        // Queue a service plugin
        Invocation * const queue(Plugin * plugin_, std::vector< Node * > input_, void * user_ = 0);

        // Cancel invocations
        void cancel();
        // Clear invocations
        void clear();
        // Is this set idle?
        bool isIdle() const;

        // Discover Invocation status
        StatusInfo status();
        double progress() const;

    signals:
        // Invocation created
        void queued(Utopia::Invocation * invocation_, Utopia::Plugin * plugin_);
        // Modfication changes
        void progressed(Utopia::Invocation * invocation_, double progress_, std::string message_);
        // Invocation ended (either returned or cancelled)
        void returned(Utopia::Invocation * invocation_, Utopia::Invocation::ErrorCode errorCode_);
        // Invocation deleted
        void deleted(Utopia::Invocation * invocation_);

        // Invocation set has become busy (a job has started running)
        void busy();
        // Invocation set has become idle (all running jobs completed)
        void idle();

    protected:
        // Deal with signals
        void _queued(Invocation * invocation_, Plugin * plugin_);
        void _progressed(Invocation * invocation_, double progress_, std::string message_);
        void _returned(Invocation * invocation_, ErrorCode errorCode_);
        void _deleted(Invocation * invocation_);

    private:
        // Invocations
        StatusInfo _invocations;

        // Convenience method for calculating overall progress
        void _recalculateProgress();

#ifdef _Utopia_THREADED
        // Service Invocation set's mutexes
        mutable struct
        {
            Mutex invocations;
            Mutex alive;
            Mutex queue;
        } _mutexes;
#endif

        // Still alive
        bool _alive;
        bool _threadLoopAlive() const;
        void _stopThreadLoop();

        // Invocation queue
        std::list< Invocation * > _queue;
        void _queueInvocation(Invocation * invocation_);
        void _unqueueInvocation(Invocation * invocation_);
        Invocation * _pop();

        // Thread pool
        boost::thread ** _threads;
        const size_t _threadCount;
        Condition _condition;
        static void * _idleLoop(void *);

        // Friend
        friend class Invocation;

    }; // class InvocationSet

} // namespace Utopia

#endif // Utopia_SERVICE_INVOCATION_SET_H
