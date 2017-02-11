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

#include <utopia2/invocation.h>
#include <utopia2/plugin.h>
#include <utopia2/invocationset.h>

namespace Utopia
{

    //
    // Class Invocation
    //

    /**
     *  \brief Service Invocation constructor.
     *
     *  Constructs a service Invocation object and sets its status to
     *  IDLE with zero progress.
     */
    Invocation::Invocation(InvocationSet * invocationSet_, Plugin * plugin_, std::vector< Node * > input_, void * user_)
        : _returned(false), _message("Queued..."), _progress(0), _errorCode(NONE), _cancelled(false), _input(input_), _plugin(plugin_), _invocationSet(invocationSet_), _user(user_)
    {
        invocationSet_->_queued(this, plugin_);
    }

    /** Service Invocation destructor. */
    Invocation::~Invocation()
    {
        this->cancel();

        this->_invocationSet->_deleted(this);

        MutexGuard guard(this->_mutexes.lifetime);
    }




    /** Get Invocation completion. */
    bool Invocation::returned() const
    {
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.returned);
#endif

        return this->_returned;
    }

    /**
     *  \brief Set Invocation completion.
     *
     *  This is a private method that can only be called by service plugins
     *  during the lifetime of the Invocation.
     */
    void Invocation::setReturned(bool returned_)
    {
//         qDebug() << this << "->setReturned() for" << this->plugin()->id();
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.returned);
#endif

        this->_returned = returned_;
    }

    /** Get Invocation status string. */
    std::string Invocation::message() const
    {
//         qDebug() << this << "->message() for" << this->plugin()->id();
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.message);
#endif

        return this->_message;
    }

    /**
     *  \brief Set Invocation status string.
     *
     *  This is a private method that can only be called by service plugins
     *  during the lifetime of the Invocation.
     */
    void Invocation::setMessage(std::string message_)
    {
//         qDebug() << this << "->setMessage() for" << this->plugin()->id();
        if (!this->cancelled())
        {
            {
#ifdef _Utopia_THREADED
                MutexGuard guard(this->_mutexes.message);
#endif

                this->_message = message_;
            }
            this->notify();
        }
    }

    /** Get Invocation progress. */
    double Invocation::progress() const
    {
//         qDebug() << this << "->progress() for" << this->plugin()->id();
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.progress);
#endif

        return this->_progress;
    }

    /**
     *  \brief Set Invocation progress.
     *
     *  This is a private method that can only be called by service plugins
     *  during the lifetime of the Invocation.
     */
    void Invocation::setProgress(double progress_)
    {
        if (!this->cancelled())
        {
//         qDebug() << this << "->setProgress() for" << this->plugin()->id();
            {
#ifdef _Utopia_THREADED
                MutexGuard guard(this->_mutexes.progress);
#endif

                // Constrain to be in range 0 - 1
                if (progress_ < 0)
                {
                    this->_progress = 0;
                }
                else if (progress_ > 1)
                {
                    this->_progress = 1;
                }
                else
                {
                    this->_progress = progress_;
                }
            }
            this->notify();
        }
    }

    /** Get Invocation error code. */
    Invocation::ErrorCode Invocation::errorCode() const
    {
//         qDebug() << this << "->errorCode() for" << this->plugin()->id();
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.errorCode);
#endif

        return this->_errorCode;
    }

    /**
     *  \brief Set Invocation error code.
     *
     *  This is a private method that can only be called by service plugins
     *  during the lifetime of the Invocation.
     */
    void Invocation::setErrorCode(Invocation::ErrorCode errorCode_)
    {
        if (!this->cancelled())
        {
//         qDebug() << this << "->setErrorCode() for" << this->plugin()->id();
#ifdef _Utopia_THREADED
            MutexGuard guard(this->_mutexes.errorCode);
#endif

            this->_errorCode = errorCode_;
        }
    }

    /**
     *  \brief Notify any updates.
     *
     *  Manually called by plugins to notify those who care about updates.
     */
    void Invocation::notify()
    {
        this->_invocationSet->_progressed(this, progress(), message());
    }

    /**
     *  \brief Query Invocation cancellation state.
     */
    bool Invocation::cancelled() const
    {
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.cancelled);
#endif

        return this->_cancelled;
    }

    /**
     *  \brief Access the user defined data member.
     */
    const void * Invocation::user() const
    {
        return this->_user;
    }

    /**
     *  \brief Cancel Invocation.
     */
    void Invocation::cancel()
    {
        bool alreadyCancelled = false;
#ifdef _Utopia_THREADED
        {
            MutexGuard guardCancelled(this->_mutexes.cancelled);
            MutexGuard guardReturned(this->_mutexes.returned);
#endif
            alreadyCancelled = this->_cancelled || this->_returned;
            if (!alreadyCancelled)
            {
#ifdef _Utopia_THREADED
                MutexGuard guardErrorCode(this->_mutexes.errorCode);
                MutexGuard guardProgress(this->_mutexes.progress);
                MutexGuard guardMessage(this->_mutexes.message);
#endif
                this->_cancelled = true;
                this->_errorCode = CANCELLED;
                this->_progress = 1.0;
                this->_message = "Cancelled by user.";
                this->_returned = true;
            }
#ifdef _Utopia_THREADED
            else
            {
                // Remove from queue if
                this->_invocationSet->_unqueueInvocation(this);
            }
        }
#endif
        if (!alreadyCancelled)
        {
            // emit returned!
            emit this->_invocationSet->_returned(this, CANCELLED);
            this->notify();
        }
    }

    /** Get service plugin for this Invocation. */
    Plugin * const Invocation::plugin() const
    {
//         qDebug() << this << "->plugin()";

        return this->_plugin;
    }

    /** Get output from this Invocation. */
    const std::vector< Node * > Invocation::output() const
    {
//         qDebug() << this << "->output()";
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.output);
#endif

        return this->_output;
    }

    /** Get input from this Invocation. */
    const std::vector< Node * > Invocation::input() const
    {
//         qDebug() << this << "->input()";

        return this->_input;
    }

    /**
     *  \brief Set output for this Invocation.
     *
     *  This is a private method that can only be called by service plugins
     *  during the lifetime of the Invocation.
     */
    void Invocation::setOutput(std::vector< Node * > output_)
    {
//         qDebug() << this << "->setOutput()";
#ifdef _Utopia_THREADED
        MutexGuard guard(this->_mutexes.output);
#endif

        this->_output = output_;
    }

    /** Invocation. */
    void Invocation::_queue()
    {
        // Invoke
        try
        {
#ifdef DEBUG
            qDebug() << " * Queuing Plugin (" << this << "):" << this->plugin()->name();
#endif
            bool success = false;
            if (!this->cancelled())
            {
                this->setMessage("Invoking...");
                success = this->plugin()->invoke(this, this->_input);
            }
#ifdef DEBUG
            qDebug() << " * Plugin returned:" << this->plugin()->name();
#endif
            if (this->cancelled())
            {
                this->setErrorCode(Invocation::CANCELLED);
                this->setMessage("Job cancelled by user.");
            }
            else if (success)
            {
                this->setErrorCode(Invocation::NONE);
            }
            else if (this->errorCode() == Invocation::NONE)
            {
                this->setErrorCode(Invocation::UNKNOWN);
            }
        }
        catch (...)
        {
            // Handle application exceptions here.
            this->setErrorCode(Invocation::CATASTROPHE);
            this->setMessage("Fatal error occurred during Plugin invocation!");
        }

        if (!this->cancelled())
        {
            // emit returned!
            this->setReturned(true);
            emit this->_invocationSet->_returned(this, this->errorCode());
            this->notify();
        }
    }

} // namespace Utopia
