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
#include <utopia2/parser.h>

#include <QFile>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QtDebug>

#include <sys/time.h>
#include <stdio.h>

namespace Utopia
{

    namespace
    {
        class registry : protected QMap< FileFormat*, Parser* >
        {
            typedef QMap< FileFormat*, Parser* > _Base;
            typedef QMapIterator< FileFormat*, Parser* > _BaseIterator;

        public:
            // Destructor
            ~registry()
                {
                    _BaseIterator format(*this);
                    while (format.hasNext())
                    {
                        format.next();
                        delete format.value();
                    }
                }

            // Static registry methods
            static Parser* find(FileFormat* format_)
                {
                    registry& reg = get();
                    if (reg.contains(format_))
                    {
                        return reg[format_];
                    }
                    else
                    {
                        return 0;
                    }
                }
            static QSet< Parser* > find(const QString& extension_)
                {
                    QSet< Parser* > matches;
                    QSet< FileFormat* > formats = FileFormat::getForExtension(extension_);
                    QSetIterator< FileFormat* > format(formats);
                    while (format.hasNext())
                    {
                        FileFormat* next = format.next();
                        Parser* parser = find(next);
                        if (parser)
                        {
                            matches += parser;
                        }
                    }
                    return matches;
                }
            static bool insert(FileFormat* format_, Parser* parser_)
                {
                    registry& reg = get();
                    if (parser_ && format_ && !reg.contains(format_))
                    {
                        reg[format_] = parser_;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            static QList< Parser* > parsers()
                {
                    return get().values();
                }
            static QList< FileFormat* > formats()
                {
                    return get().keys();
                }

        private:
            // Constructor
            registry()
                : _Base()
                {
                }

            // Static registry get
            static registry& get()
                {
                    static registry reg;
                    return reg;
                }

        }; // class registry

    } // Anonymous namespace



    Parser::Context::Context(const Parser* parser_)
        : _parser(parser_), _model(0), _errorCode(None), _errorLine(0), _errorCharacter(0)
    {}

    const Parser* Parser::Context::parser() const
    {
        return this->_parser;
    }

    /** Return model node. */
    Node* Parser::Context::model() const
    {
        return this->_model;
    }

    /** Return error code. */
    Parser::ErrorCode Parser::Context::errorCode() const
    {
        return this->_errorCode;
    }

    /** Return error line. */
    size_t Parser::Context::errorLine() const
    {
        return this->_errorLine;
    }

    /** Return error character. */
    size_t Parser::Context::errorCharacter() const
    {
        return this->_errorCharacter;
    }

    /** Return message. */
    QString Parser::Context::message() const
    {
        return this->_message;
    }

    /** Return warnings. */
    QList< Parser::Warning > Parser::Context::warnings() const
    {
        return this->_warnings;
    }

    /** Set model node. */
    void Parser::Context::setModel(Node* model_)
    {
        this->_model = model_;
//        qDebug() << "** model = " << this->_model;
    }

    /** Set error code. */
    void Parser::Context::setErrorCode(Parser::ErrorCode errorCode_)
    {
        this->_errorCode = errorCode_;
    }

    /** Set error line. */
    void Parser::Context::setErrorLine(size_t errorLine_)
    {
        this->_errorLine = errorLine_;
    }

    /** Set error character. */
    void Parser::Context::setErrorCharacter(size_t errorCharacter_)
    {
        this->_errorCharacter = errorCharacter_;
    }

    /** Set message. */
    void Parser::Context::setMessage(const QString& message_)
    {
        this->_message = message_;
    }

    /** Add warning. */
    void Parser::Context::addWarning(const QString& message_, size_t line_, size_t character_)
    {
        this->_warnings.push_back(Warning(message_, line_, character_));
    }



    /** Parse! */
    Parser::Context Parser::parse(QIODevice& stream_) const
    {
        Parser::Context ctx(this);
        ctx.setModel(this->parse(ctx, stream_));
        return ctx;
    }

    /** Default acceptance. */
    Parser::Acceptance Parser::accepts(QIODevice& input_) const
    {
        return Parser::Accepted;
    }

    /** Default format list */
    QSet< FileFormat* > Parser::formats() const
    {
        QSet< FileFormat* > formats;
        return formats;
    }

    void Parser::registerFormat(FileFormat* format_, Parser* parser_)
    {
        registry::insert(format_, parser_);
    }

    /**
     *  \brief Get Parser for a given FileFormat.
     */
    Parser* Parser::get(FileFormat* format_)
    {
        return registry::find(format_);
    }

    /**
     *  \brief Get Parser for a given file extension.
     */
    QSet< Parser* > Parser::get(const QString& extension_)
    {
        QSet< Parser* > parsers;
        QList< FileFormat* > formats = registry::formats();
        for (int i = 0; i < formats.size(); ++i)
        {
            FileFormat* format = formats.at(i);
            if (format->contains(extension_))
            {
                parsers.insert(registry::find(format));
            }
        }
        return parsers;
    }

    /**
     *  \brief Get all available Parsers.
     */
    QSet< Parser* > Parser::all()
    {
        QSet< Parser* > all;
        QList< Parser* > parsers = registry::parsers();
        for (int i = 0; i < parsers.size(); ++i)
        {
            all.insert(parsers.at(i));
        }
        return all;
    }

    /** Parse stream using given file format. */
    Parser::Context parse(QIODevice& stream_, FileFormat* fileFormat_)
    {
        Parser* parser = Parser::get(fileFormat_);
        return parser ? parser->parse(stream_) : Parser::Context(0);
    }

    /** Load file using given file format. */
    Parser::Context load(const QString& fileName_, FileFormat* fileFormat_)
    {
        QFile file(fileName_);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (fileFormat_ == 0)
        {
            QSet< FileFormat* > formats = FileFormat::getForExtension(fileName_.section(".", -1, -1));
            if (formats.count() != 1)
            {
                // FIXME cannot distringuish file format from extension
                return Parser::Context(0);
            }
            else
            {
                fileFormat_ = *formats.begin();
            }
        }
        return parse(file, fileFormat_);
    }

} // namespace Utopia

UTOPIA_DEFINE_EXTENSION_CLASS(Utopia::Parser)
