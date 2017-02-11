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

#ifndef Utopia_INITIALIZER_H
#define Utopia_INITIALIZER_H

#include <utopia2/config.h>

#include <utopia2/extension.h>


#include <QString>
#include <QVector>
#include <iostream>

#include <QObject>

namespace Utopia
{

    /**
     *  \class Initializer
     *  \brief The abstract Initializer class.
     */
    class LIBUTOPIA_API Initializer : public QObject
    {
        Q_OBJECT

    public:
        typedef Initializer API;

        typedef enum
        {
            None = 0,
            Fatal,
            Unknown
        } ErrorCode;

        // Constructor
        Initializer();
        // Virtual Destructor
        virtual ~Initializer() {}

        // Initialise and finalise methods
        virtual void init() {}
        virtual void postInit() {}
        virtual void final() {}

        // Helper methods
        virtual QString description() = 0;
        virtual QVector< QString > prerequisites();

        // Reporting
        QString message() const;
        ErrorCode errorCode() const;

        static void cleanup();

    Q_SIGNALS:
        // Progress
        void messageChanged(QString message_);

    protected:
        // Set message
        void setMessage(const QString& message_);
        // Set error code
        void setErrorCode(ErrorCode errorCode_);

    private:
        // Message string
        QString _message;
        // Error code
        ErrorCode _errorCode;

    }; /* class Initializer */

} /* namespace Utopia */

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA, Utopia::Initializer)

#endif /* Utopia_INITIALIZER_H */
