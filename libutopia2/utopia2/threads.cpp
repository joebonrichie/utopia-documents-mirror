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

#include <utopia2/threads.h>
#include <utopia2/hashmap.h>

#ifdef _Utopia_THREADED

#include <iostream>
#include <fstream>
#include <map>

#include <QtDebug>

namespace Utopia
{
    //
    // class Mutex
    //

    /** Create Mutex. */
    Mutex::Mutex()
        : _errorCode(SUCCESS)
    {
        // Initialise Mutex
        try
        {
            _mutex = new boost::mutex;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Mutex initialisation error.";
            this->_errorCode = FAILURE;
            qDebug() << this->_errorString;
        }
    }

    /** Destroy Mutex. */
    Mutex::~Mutex()
    {
        // Destroy Mutex
        try
        {
            delete _mutex;
        }
        catch (boost::thread_resource_error)
        {
            // FIXME: More abortive error handling needed here
            this->_errorString = "boost::thread Mutex destruction error.";
            qDebug() << this->_errorString;
        }
    }

    /** Lock Mutex. */
    int Mutex::lock()
    {
        // Guard against recursion
        if (++this->_depth() > 1)
        {
            return this->_errorCode = SUCCESS;
        }

        try
        {
            _mutex->lock();
            return this->_errorCode = SUCCESS;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Mutex locking error.";
            qDebug() << this->_errorString;
            return this->_errorCode = FAILURE;
        }
    }

    /** Attempt to lock Mutex. */
    int Mutex::tryLock()
    {
        // Guard against recursion
        if (++this->_depth() > 1)
        {
            return this->_errorCode = SUCCESS;
        }

        try
        {
            if (_mutex->try_lock())
            {
                return this->_errorCode = SUCCESS;
            }
            else
            {
                return this->_errorCode = BUSY;
            }
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Mutex locking error.";
            qDebug() << this->_errorString;
            return this->_errorCode = FAILURE;
        }
    }

    /** Unlock Mutex. */
    int Mutex::unlock()
    {
        // Guard against recursion
        if (--this->_depth() > 0)
        {
            return this->_errorCode = SUCCESS;
        }

        try
        {
            _mutex->unlock();
            return this->_errorCode = SUCCESS;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Mutex unlocking error.";
            qDebug() << this->_errorString;
            return this->_errorCode = FAILURE;
        }
    }

    /** Get Mutex error code. */
    int Mutex::errorCode()
    { return this->_errorCode; }

    /** Get Mutex error string. */
    QString Mutex::errorString()
    { return this->_errorString; }

    /** Get this thread's depth. */
    unsigned int& Mutex::_depth()
    {
        if (_tsd.get() == 0)
        {
            _tsd.reset(new std::map< Mutex *, unsigned int >);
        }

        std::map< Mutex *, unsigned int > * mutexMap = _tsd.get();

        // Is this Mutex already present?
        if (mutexMap->find(this) == mutexMap->end())
        {
            return ((*mutexMap)[this] = 0);
        }
        else
        {
            return (*mutexMap)[this];
        }
    }


    //
    // class MutexGuard
    //

    /** Construct guard. */
    MutexGuard::MutexGuard(Mutex& mutex_, bool lock_)
        : _mutex(mutex_)
    {
        if (lock_)
        {
            this->_mutex.lock();
        }
    }

    /** Destroy guard, and unlock Mutex. */
    MutexGuard::~MutexGuard()
    { this->_mutex.unlock(); }


    //
    // class Semaphore
    //

    /** Create Semaphore. */
    Semaphore::Semaphore(unsigned int size_)
        : _size(size_), _errorCode(SUCCESS)
    {
        if (this->_mutex.errorCode() == Mutex::FAILURE)
        {
            this->_errorString = "boost::thread Semaphore Mutex initialisation error.";
            this->_errorCode = FAILURE;
        }

        /* Initialise Condition */
        try
        {
            _condition = new boost::condition_variable;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Semaphore Condition initialisation error.";
            this->_errorCode = FAILURE;
        }
    }

    /** Destroy Semaphore. */
    Semaphore::~Semaphore()
    {
        try
        {
            delete _condition;
        }
        catch (boost::thread_resource_error)
        {
            // FIXME: More abortive error handling needed here
            this->_errorString = "boost::thread Semaphore Condition destruction error.";
            this->_errorCode = FAILURE;
        }
    }

    /** Lock Semaphore. */
    int Semaphore::lock()
    {
        // Lock this Mutex
        if (this->_mutex.lock() != Mutex::SUCCESS) {
            this->_errorString = this->_mutex.errorString();
            return this->_errorCode = FAILURE;
        }
        MutexGuard guard(this->_mutex, false);

        // Semaphore Condition
        while (this->_size == 0)
        {
            boost::unique_lock< boost::mutex > l(*this->_mutex._mutex);
            this->_condition->wait(l);
        }

        // Decrement Semaphore
        --this->_size;

        return this->_errorCode = SUCCESS;
    }

    /** Unlock Semaphore. */
    int Semaphore::unlock()
    {
        // Lock this Mutex
        if (this->_mutex.lock() != Mutex::SUCCESS) {
            this->_errorString = this->_mutex.errorString();
            return this->_errorCode = FAILURE;
        }
        MutexGuard guard(this->_mutex, false);

        // Increment Semaphore
        ++this->_size;

        // Broadcast signal for Condition
        this->_condition->notify_all();

        return this->_errorCode = SUCCESS;
    }

    /** Get Semaphore error code. */
    int Semaphore::errorCode()
    { return this->_errorCode; }

    /** Get Semaphore error string. */
    QString Semaphore::errorString()
    { return this->_errorString; }

    //
    // class SemaphoreGuard
    //

    /** Construct guard. */
    SemaphoreGuard::SemaphoreGuard(Semaphore& semaphore_, bool lock_)
        : _semaphore(semaphore_)
    {
        if (lock_)
        {
            this->_semaphore.lock();
        }
    }

    /** Destroy guard, and unlock Semaphore. */
    SemaphoreGuard::~SemaphoreGuard()
    { this->_semaphore.unlock(); }

    //
    // class Condition
    //

    /** Construct Condition. */
    Condition::Condition()
        : _errorCode(SUCCESS)
    {
        if (this->_mutex.errorCode() == Mutex::FAILURE)
        {
            this->_errorString = "boost::thread Condition Mutex initialisation error.";
            this->_errorCode = FAILURE;
        }

        /* Initialise Condition */
        try
        {
            _condition = new boost::condition_variable;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Condition initialisation error.";
            this->_errorCode = FAILURE;
        }
    }

    /** Destroy Condition. */
    Condition::~Condition()
    {
        try
        {
            delete _condition;
        }
        catch (boost::thread_resource_error)
        {
            // FIXME: More abortive error handling needed here
            this->_errorString = "boost::thread Condition destruction error.";
            this->_errorCode = FAILURE;
        }
    }

    /** Broadcast this Condition. */
    int Condition::broadcast()
    {
        this->_condition->notify_all();
        return this->_errorCode = SUCCESS;
    }

    /** Signal this Condition. */
    int Condition::signal()
    {
        this->_condition->notify_one();
        return this->_errorCode = SUCCESS;
    }

    /** Wait on this Condition. */
    int Condition::wait()
    {
        // Guard Condition
        MutexGuard guard(this->_mutex);

        try
        {
            boost::unique_lock< boost::mutex > l(*this->_mutex._mutex);
            this->_condition->wait(l);
            return this->_errorCode = SUCCESS;
        }
        catch (boost::thread_resource_error)
        {
            this->_errorString = "boost::thread Condition wait error.";
            qDebug() << this->_errorString;
            return this->_errorCode = FAILURE;
        }
    }

    /** Get Condition error code. */
    int Condition::errorCode()
    { return this->_errorCode; }

    /** Get Condition error string. */
    QString Condition::errorString()
    { return this->_errorString; }

} /* namespace Utopia */

#endif /* _USM_THREADS */
