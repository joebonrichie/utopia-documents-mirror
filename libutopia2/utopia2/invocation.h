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

#ifndef Utopia_SERVICE_INVOCATION_H
#define Utopia_SERVICE_INVOCATION_H

#include <utopia2/config.h>
#include <utopia2/threads.h>
#include <string>
#include <vector>

namespace Utopia
{

    class Node;
    class InvocationSet;
    class Plugin;

    /**
     *  \class Invocation
     *
     *  Encapsulates a service Invocation (a job).
     */
    class LIBUTOPIA_API Invocation
    {
    public:
        // Error codes
        typedef enum
        {
            NONE = 0,
            NO_RESULT,          // No results returned
            UNKNOWN_SERVICE,    // Unknown service description
            SERVICE_FAILURE,    // Remote service failure
            CATASTROPHE,        // Thread crashed!
            CANCELLED,          // Thread cancelled
            UNKNOWN
        } ErrorCode;

        // Destructor
        ~Invocation();

        // Get and set methods
        Plugin* const plugin() const;

        bool returned() const;
        std::string message() const;
        double progress() const;
        ErrorCode errorCode() const;
        const std::vector< Node * > input() const;
        const std::vector< Node * > output() const;
        bool cancelled() const;
        const void * user() const;

        void setMessage(std::string message_);
        void setProgress(double progress_);
        void setErrorCode(ErrorCode errorCode_);
        void setOutput(std::vector< Node * > output_);

        void notify();
        void cancel();

    private:
        // Private constructor (can only be instatiated by friends)
        Invocation(InvocationSet * invocationSet_, Plugin * plugin_, std::vector< Node * > input_, void * user_ = 0);

        // Set returned flag
        void setReturned(bool returned_);

        // Job returned?
        bool _returned;
        // Job status string
        std::string _message;
        // Job progress (floating point value between 0 and 1
        double _progress;
        // Job error code
        ErrorCode _errorCode;
        // Cancelled?
        bool _cancelled;

        // Job input
        std::vector< Node * > _input;
        // Job output
        std::vector< Node * > _output;

        // Job's service description
        Plugin * _plugin;

        // Owning set
        InvocationSet * _invocationSet;

        // Userdef
        void * _user;

#ifdef _Utopia_THREADED
        // Service Invocation's mutexes
        mutable struct
        {
            Mutex returned;
            Mutex message;
            Mutex progress;
            Mutex errorCode;
            Mutex output;
            Mutex cancelled;
            Mutex lifetime;
        } _mutexes;
#endif

        friend class InvocationSet;

        // Invocation arguments
        struct _invocation
        {
            Plugin * plugin;
            Invocation * invocation;
            std::vector< Node * > input;
        }; /* struct _invocation */

        // Static Invocation functions
        // Queue
        void _queue();

    }; // class Invocation

} // namespace Utopia

#endif // Utopia_SERVICE_INVOCATION_H
