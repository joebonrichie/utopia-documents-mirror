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

//#include <utopia2/extension_impl.h>
#include <utopia2/initializer.h>

#include <QStack>
#include <QString>

namespace Utopia
{
    namespace
    {
        class Registry
        {
        public:
            // Destructor for cleanup
            ~Registry()
            {
                cleanup();
            }

            static void cleanup()
            {
                // In reverse order, clean up
                while (!get().empty())
                {
                    Initializer* obj = get().top();
                    obj->final();
                    delete obj;
                    get().pop();
                }
            }

            // Get Registry instance
            static QStack< Initializer* >& get()
            {
                static Registry reg;
                return reg._initializers;
            }

        private:
            // Initializer List
            QStack< Initializer* > _initializers;

        }; /* class Registry */
    }

    /** Superclass constructor appends the Initializer to the Registry stack. */
    Initializer::Initializer()
        : _errorCode(None)
    {
        Registry::get().push(this);
    }

    /** Return no prerequisites by default. */
    QVector< QString > Initializer::prerequisites()
    {
        QVector< QString > requires;
        return requires;
    }

    /** Return message. */
    QString Initializer::message() const
    {
        return _message;
    }

    /** Return error code. */
    Initializer::ErrorCode Initializer::errorCode() const
    {
        return _errorCode;
    }

    /** Set message. */
    void Initializer::setMessage(const QString& message_)
    {
        bool changed = (this->_message != message_);
        if (changed)
        {
            this->_message = message_;
            emit messageChanged(this->_message);
        }
    }

    /** Set error code. */
    void Initializer::setErrorCode(Initializer::ErrorCode errorCode_)
    {
        _errorCode = errorCode_;
    }

    void Initializer::cleanup()
    {
        Registry::cleanup();
    }

} // namespace Utopia

UTOPIA_DEFINE_EXTENSION_CLASS(Utopia::Initializer)
