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

#ifndef Utopia_THREADS_H
#define Utopia_THREADS_H

#include <utopia2/config.h>

#ifdef _Utopia_THREADED

#include <map>
#include <boost/thread.hpp>

#include <QString>

namespace Utopia
{
    // Forward declarations...
    class Semaphore;

    /**
     *  \class Mutex
     *
     *  Abstract Mutex class.
     */
    class LIBUTOPIA_API Mutex
    {
    public:
        typedef enum
        {
            SUCCESS = 0,
            FAILURE,
            BUSY
        } ErrorCode;

        // Constructor
        Mutex();
        // Destructor
        ~Mutex();

        // Mutex management
        int lock();
        int tryLock();
        int unlock();

        // Error handling
        int errorCode();
        QString errorString();

    private:
        // Actual Mutex...
        boost::mutex * _mutex;
        boost::thread_specific_ptr< std::map< Mutex *, unsigned int > > _tsd;

        // Mutex depth
        unsigned int& _depth();

        // Error handling
        int _errorCode;
        QString _errorString;

        // For Semaphore handling...
        friend class Semaphore;
        // For Condition handling...
        friend class Condition;

    }; /* class Mutex */

    /**
     *  \class MutexGuard
     *
     *  Mutex guard class.
     */
    class LIBUTOPIA_API MutexGuard
    {
    public:
        // Constructor
        MutexGuard(Mutex& mutex_, bool lock_ = true);
        // Destructor
        ~MutexGuard();

    private:
        // Actual Mutex...
        Mutex& _mutex;

    }; /* class MutexGuard */

    /**
     *  \brief Semaphore
     *
     *  Abstract Semaphore class.
     */
    class LIBUTOPIA_API Semaphore
    {
    public:
        typedef enum
        {
            SUCCESS = 0,
            FAILURE,
            BUSY
        } ErrorCode;

        // Constructor
        Semaphore(unsigned int size_);
        // Destructor
        ~Semaphore();

        // Semaphore management
        int lock();
        int unlock();

        // Error handling
        int errorCode();
        QString errorString();

    private:
        // Mutex for this Semaphore
        Mutex _mutex;
        // Blocking Condition
        boost::condition_variable * _condition;
        // Semaphore size
        unsigned int _size;

        // Error handling
        int _errorCode;
        QString _errorString;

    }; /* class Semaphore */

    /**
     *  \class SemaphoreGuard
     *
     *  Mutex guard class.
     */
    class LIBUTOPIA_API SemaphoreGuard
    {
    public:
        // Constructor
        SemaphoreGuard(Semaphore& semaphore_, bool lock_ = true);
        // Destructor
        ~SemaphoreGuard();

    private:
        // Actual Semaphore...
        Semaphore& _semaphore;

    }; /* class SemaphoreGuard */

    /**
     *  \class Condition
     *
     *  Wait Condition.
     */
    class LIBUTOPIA_API Condition
    {
    public:
        typedef enum
        {
            SUCCESS = 0,
            FAILURE,
            BUSY
        } ErrorCode;

        // Constructor
        Condition();
        // Destructor
        ~Condition();

        // Condition management
        int broadcast();
        int signal();
        int wait();

        // Error handling
        int errorCode();
        QString errorString();

    private:
        // Condition Mutex
        Mutex _mutex;
        // Actual Condition
        boost::condition_variable * _condition;

        // Error handling
        int _errorCode;
        QString _errorString;

    }; /* class Condition */

} /* namespace Utopia */

#endif /* _Utopia_THREADED */

#endif /* Utopia_THREADS_H */
