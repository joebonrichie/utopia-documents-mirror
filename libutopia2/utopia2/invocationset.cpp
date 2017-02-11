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

#include <utopia2/invocationset.h>
#include <utopia2/invocation.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <QMetaType>
#include <QtDebug>

#include <iostream>

namespace Utopia
{

    //
    // Class InvocationSet
    //

    /**
     *  \brief Service Invocation set constructor.
     *
     *  Constructs an empty service Invocation set.
     */
    InvocationSet::InvocationSet(size_t threadCount_)
        : QObject(), _alive(true), _threadCount(threadCount_)
    {
#ifdef DEBUG
        qDebug() << "InvocationSet()" << this;
#endif
#if defined(_Utopia_THREADED) && defined(DEBUG)
//         qDebug() << "PTHREAD_KEYS_MAX =" << PTHREAD_KEYS_MAX;
//         qDebug() << "InvocationSet() =" << this;
#endif
        qRegisterMetaType< Utopia::Invocation::ErrorCode >("Utopia::Invocation::ErrorCode");

        // Set up thread pool
        this->_threads = new boost::thread*[this->_threadCount];
        for (size_t i = 0; i < this->_threadCount; ++i)
        {
            this->_threads[i] = new boost::thread(boost::bind(_idleLoop, this));
        }
    }

    /**
     *  \brief Queue a particular service plugin.
     *  \param plugin_ Description of service to queue.
     *  \param input_ Input on which to queue service.
     *  \param completion_callback_ Callback function for completed jobs.
     *  \param failure_callback_ Callback function for failed jobs.
     */
    Invocation* const InvocationSet::queue(Plugin* plugin_, std::vector< Node* > input_, void * user_)
    {
#ifdef _Utopia_THREADED
        // Guard Invocation queue
        this->_mutexes.invocations.lock();
#endif

        // Check if we were idle...
        if (this->_invocations.running.size() == 0)
        {
            emit busy();
        }

        // Queue the service by creating a Invocation object
        Invocation* invocation = new Invocation(this, plugin_, input_, user_);

        // Add it to the running queue
        this->_invocations.running.insert(invocation);

#ifdef _Utopia_THREADED
        // Unlock Invocation queue
        this->_mutexes.invocations.unlock();

        // Add to set's Invocation queue
        this->_queueInvocation(invocation);
#else
        // Queue in serial (blocks until returned)
        invocation->_queue();
#endif

        // Recalculate progress while we have the Mutex locked...
        this->_recalculateProgress();

        return invocation;
    }

    /** Cancel invocations. */
    void InvocationSet::cancel()
    {
#ifdef _Utopia_THREADED
        {
            MutexGuard(this->_mutexes.queue);
#endif
            // Clear queued invocations
            this->_queue.clear();
#ifdef _Utopia_THREADED
        }
#endif

        std::set< Invocation* > all;
#ifdef _Utopia_THREADED
        {
            MutexGuard(this->_mutexes.invocations);
#endif
            // Build temporary List of invocations
            all = this->_invocations.running;
#ifdef _Utopia_THREADED
        }
#endif

        // Cancel all invocations (thereby halting them)
        std::set< Invocation* >::iterator invocation_iter = all.begin();
        std::set< Invocation* >::iterator invocation_end = all.end();
        for (; invocation_iter != invocation_end; ++invocation_iter)
        {
            (*invocation_iter)->cancel();
        }
    }

    /** Clear invocations. */
    void InvocationSet::clear()
    {
        // Cancel jobs
        this->cancel();

        std::set< Invocation* > all;
#ifdef _Utopia_THREADED
        {
            MutexGuard(this->_mutexes.invocations);
#endif
            // Build temporary List of invocations
            all.insert(this->_invocations.completed.begin(), this->_invocations.completed.end());
            all.insert(this->_invocations.failed.begin(), this->_invocations.failed.end());
            this->_invocations.running.clear();
            this->_invocations.completed.clear();
            this->_invocations.failed.clear();
            this->_invocations.progress = 0;
#ifdef _Utopia_THREADED
        }
#endif

        // Delete all invocations
        std::set< Invocation* >::iterator invocation_iter = all.begin();
        std::set< Invocation* >::iterator invocation_end = all.end();
        for (; invocation_iter != invocation_end; ++invocation_iter)
        {
            delete *invocation_iter;
        }
    }

    /** Is this service Invocation set idle? */
    bool InvocationSet::isIdle() const
    {
#ifdef _Utopia_THREADED
        MutexGuard(this->_mutexes.invocations);
#endif

        return this->_invocations.running.empty();
    }

    /** Service Invocation set destructor. */
    InvocationSet::~InvocationSet()
    {
#ifdef DEBUG
        qDebug() << "~InvocationSet()" << this;
#endif
        // Stop thread loop
        this->_stopThreadLoop();

        // Awaken threads
        this->_condition.broadcast();

        // Clean up invocations
        this->clear();

        // Join the threads
        for (size_t i = 0; i < this->_threadCount; ++i)
        {
#ifdef DEBUG
            qDebug() << " * Joining thread";
#endif
            this->_threads[i]->join();
            delete this->_threads[i];
#ifdef DEBUG
            qDebug() << " * Thread joined";
#endif
        }

        // Delete threads
        delete this->_threads;
    }

    /** Retrieve Invocation set status. */
    InvocationSet::StatusInfo InvocationSet::status()
    {
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.invocations);
#endif

        return this->_invocations;
    }

    /** Retrieve Invocation set progress. */
    double InvocationSet::progress() const
    {
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.invocations);
#endif

        return this->_invocations.progress;
    }

    /** Deal with created invocations. */
    void InvocationSet::_queued(Invocation* invocation_, Plugin* plugin_)
    {
        this->_recalculateProgress();

        emit queued(invocation_, plugin_);
    }

    /** Deal with modified invocations. */
    void InvocationSet::_progressed(Invocation* invocation_, double progress_, std::string message_)
    {
        this->_recalculateProgress();

        emit progressed(invocation_, progress_, message_);
    }

    /** Deal with returned invocations. */
    void InvocationSet::_returned(Invocation* invocation_, ErrorCode errorCode_)
    {
        // Used to check idle status
        bool nowIdle = false;

#ifdef _Utopia_THREADED
        {
            MutexGuard guard(this->_mutexes.invocations);
#endif
            if (errorCode_ == Invocation::NONE)
            {
                // Remove from running List and append to completed List
                if (this->_invocations.running.erase(invocation_) > 0)
                {
                    this->_invocations.completed.insert(invocation_);
                }
            }
            else
            {
                // Remove from running List and append to failed List
                if (this->_invocations.running.erase(invocation_) > 0)
                {
                    this->_invocations.failed.insert(invocation_);
                }
            }

            // Check if we are now idle...
            nowIdle = (this->_invocations.running.size() == 0);
#ifdef _Utopia_THREADED
        }
#endif

        emit returned(invocation_, errorCode_);

        if (nowIdle) {
            emit idle();
        }
    }

    /** Deal with deleted invocations. */
    void InvocationSet::_deleted(Invocation* invocation_)
    {
#ifdef _Utopia_THREADED
        this->_mutexes.invocations.lock();
#endif
        // Remove this Invocation
        this->_invocations.running.erase(invocation_);
        this->_invocations.completed.erase(invocation_);
        this->_invocations.failed.erase(invocation_);
#ifdef _Utopia_THREADED
        this->_mutexes.invocations.unlock();
#endif

        // Recalculate progress
        this->_recalculateProgress();

        emit deleted(invocation_);
    }

    /** Calculate overall progress. */
    void InvocationSet::_recalculateProgress()
    {
#ifdef _Utopia_THREADED
        this->_mutexes.invocations.lock();
#endif
        // Volatile variable access
        double progress = 0;
        size_t running = this->_invocations.running.size();
        size_t completed = this->_invocations.completed.size();
        size_t failed = this->_invocations.failed.size();
        std::set< Invocation* > running_copy(this->_invocations.running);
#ifdef _Utopia_THREADED
        this->_mutexes.invocations.unlock();
#endif

        // Guard for divide by zero!
        if (running + completed + failed == 0)
        {
            // Set progress to 100%
            progress = 1;
        }
        else
        {
            // Just take all completed / failed jobs to be 1
            progress += completed;
            progress += failed;

            // Add up all the running jobs' progresses
            std::set< Invocation* >::iterator invocation_iter = running_copy.begin();
            std::set< Invocation* >::iterator invocation_end = running_copy.end();
            for (; invocation_iter != invocation_end; ++invocation_iter)
            {
                progress += (*invocation_iter)->progress();
            }

            // Scale back down to unit
            progress /= (double) (running + completed + failed);
        }

#ifdef _Utopia_THREADED
        this->_mutexes.invocations.lock();
#endif
        // Volatile variable access
        this->_invocations.progress = progress;
#ifdef _Utopia_THREADED
        this->_mutexes.invocations.unlock();
#endif
    }

    /** Is this set still alive? */
    bool InvocationSet::_threadLoopAlive() const
    {
        MutexGuard guard(this->_mutexes.alive);

        return this->_alive;
    }

    /** Set alive value */
    void InvocationSet::_stopThreadLoop()
    {
        MutexGuard guard(this->_mutexes.alive);

        this->_alive = false;
    }

    /** Queue Invocation ready for a thread to become available. */
    void InvocationSet::_queueInvocation(Invocation* invocation_)
    {
        MutexGuard guard(this->_mutexes.queue);

        this->_queue.push_back(invocation_);
        this->_condition.signal();
    }

    /** Unqueue Invocation. */
    void InvocationSet::_unqueueInvocation(Invocation* invocation_)
    {
        MutexGuard guard(this->_mutexes.queue);

        this->_queue.remove(invocation_);
    }

    /** Pop a job signature from the queue. */
    Invocation* InvocationSet::_pop()
    {
        MutexGuard guard(this->_mutexes.queue);

        if (this->_queue.size() > 0)
        {
            Invocation* Invocation = this->_queue.front();
            this->_queue.pop_front();
            return Invocation;
        }
        else
        {
            return 0;
        }
    }

    /** Static thread loop. */
    void* InvocationSet::_idleLoop(void* invocationSet_)
    {
        InvocationSet* invocationSet = (InvocationSet*) invocationSet_;

        while (invocationSet->_threadLoopAlive())
        {
            Invocation* invocation = invocationSet->_pop();
            if (invocation == 0)
            {
                invocationSet->_condition.wait();
            }
            else
            {
                MutexGuard guard(invocation->_mutexes.lifetime);
                // Queue job
                invocation->_queue();
            }
        }

        return 0;
    }

} // namespace Utopia
