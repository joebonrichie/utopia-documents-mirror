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

#ifndef Utopia_SERIALIZER_H
#define Utopia_SERIALIZER_H

#include <utopia2/config.h>
#include <utopia2/enums.h>
#include <utopia2/extension.h>

#include <utopia2/fileformat.h>

#include <utopia2/library.h>

#include <QSet>
#include <QString>
#include <QIODevice>

namespace Utopia
{
    // Forward declarations
    class Node;

    /**
     *  \class Serializer
     *  \brief The abstract Serializer class.
     */
    class LIBUTOPIA_API Serializer
    {
    public:
        typedef Serializer API;

        typedef enum
        {
            None = 0,
            Unknown
        } ErrorCode;

        class LIBUTOPIA_API Context
        {
        public:
            // Constructor
            Context(const Serializer* serializer_);

            // Context
            const Serializer* serializer() const;

            // Reporting
            ErrorCode errorCode() const;
            QString message() const;

            // Set error code
            void setErrorCode(ErrorCode errorCode_);
            // Set message
            void setMessage(const QString& message_);

        private:
            // Serializer used
            const Serializer* _serializer;

            // Message string
            QString _message;
            // Error code
            ErrorCode _errorCode;

            friend class Serializer;
        };

        Serializer(FormatCategory formatCategory_ = UnknownFormat)
            : _errorCode(None), _formatCategory(formatCategory_)
        {}
        // Virtual Destructor
        virtual ~Serializer() {}

        // Serialize method
        Context serialize(QIODevice& stream_, Node* node_) const;
        virtual bool serialize(Context& ctx, QIODevice& stream_, Node* node_) const = 0;
        virtual QString description() const = 0;
        virtual QSet< FileFormat* > formats() const;

        // Reporting
        FormatCategory formatCategory() const;

        // Static serializer methods
        static void init();
        static Serializer* get(FileFormat* format_);
        static void registerFormat(FileFormat* format_, Serializer* serializers_);
        static QSet< Serializer* > get(const QString& extension_);
        static QSet< Serializer* > all();

    private:
        // Message string
        QString _message;
        // Error code
        ErrorCode _errorCode;
        // Format Category
        FormatCategory _formatCategory;

    }; /* class Serializer */

    LIBUTOPIA_API Serializer::Context serialize(QIODevice& stream_, Node* node_, FileFormat* fileFormat_);
    LIBUTOPIA_API Serializer::Context save(const QString& fileName_, Node* node_, FileFormat* fileFormat_ = 0);

} /* namespace Utopia */

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA, Utopia::Serializer)

#endif /* Utopia_SERIALIZER_H */
