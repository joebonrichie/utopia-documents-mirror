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

#ifndef Utopia_PARSER_H
#define Utopia_PARSER_H

#include <utopia2/config.h>

#include <utopia2/enums.h>

#include <utopia2/extension.h>

#include <utopia2/fileformat.h>

#include <QSet>
#include <QString>
#include <QIODevice>

namespace Utopia
{
    // Forward declarations
    class Node;

    /**
     *  \class Parser
     *  \brief The abstract Parser class.
     */
    class LIBUTOPIA_API Parser
    {
    public:
        typedef Parser API;

        typedef enum
        {
            None = 0,
            StreamError,
            StreamEmpty,
            SyntaxError,
            UnexpectedEof,
            UnexpectedEol,
            Incapable,
            Unknown
        } ErrorCode;

        typedef enum
        {
            Accepted = 0,
            Rejected,
            Unchecked
        } Acceptance;

        class LIBUTOPIA_API Warning
        {
        public:
            // Constructor
            Warning(QString message_, size_t line_ = 0, size_t character_ = 0)
                : message(message_), line(line_), character(character_) {};

            QString message;
            size_t line;
            size_t character;
        };

        class LIBUTOPIA_API Context
        {
        public:
            // Constructor
            Context(const Parser* parser_);

            // Context
            const Parser* parser() const;

            // Result
            Node* model() const;

            // Reporting
            ErrorCode errorCode() const;
            size_t errorLine() const;
            size_t errorCharacter() const;
            QString message() const;
            QList< Warning > warnings() const;

            // Set error code
            void setErrorCode(ErrorCode errorCode_);
            // Set error line
            void setErrorLine(size_t errorLine_);
            // Set error character
            void setErrorCharacter(size_t errorCharacter_);
            // Set message
            void setMessage(const QString& message_);
            // Add warning
            void addWarning(const QString& message_, size_t line_ = 0, size_t character_ = 0);

        private:
            // Parser used
            const Parser* _parser;
            // Model node
            Node* _model;

            // Message string
            QString _message;
            // Error code
            ErrorCode _errorCode;
            // Error line number
            size_t _errorLine;
            // Error character position
            size_t _errorCharacter;
            // Warning messages
            QList< Warning > _warnings;

            // Set model node
            void setModel(Node* node);

            friend class Parser;
        };

        // Virtual Destructor
        Parser() {};
        virtual ~Parser() {};

        // Parse method
        Context parse(QIODevice& stream_) const;
        virtual Node* parse(Context& ctx, QIODevice& stream_) const = 0;
        virtual Acceptance accepts(QIODevice& stream_) const;
        virtual QString description() const = 0;
        virtual QSet< FileFormat* > formats() const;

        // Static parser methods
        static Parser* get(FileFormat* format_);
        static void registerFormat(FileFormat* format_, Parser* parser_);
        static QSet< Parser* > get(const QString& extension_);
        static QSet< Parser* > all();

    }; /* class Parser */

    LIBUTOPIA_API Parser::Context parse(QIODevice& stream_, FileFormat* fileFormat_);
    LIBUTOPIA_API Parser::Context load(const QString& fileName_, FileFormat* fileFormat_ = 0);

} /* namespace Utopia */

UTOPIA_DECLARE_EXTENSION_CLASS(LIBUTOPIA, Utopia::Parser)

#endif /* Utopia_PARSER_H */
