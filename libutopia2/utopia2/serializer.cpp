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
#include <utopia2/serializer.h>

#include <QFile>
#include <QMap>

namespace Utopia
{

    namespace
    {
        class registry : protected QMap< FileFormat*, Serializer* >
        {
            typedef QMap< FileFormat*, Serializer* > _Base;
            typedef QMapIterator< FileFormat*, Serializer* > _BaseIterator;

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
            static Serializer* find(FileFormat* format_)
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
            static QSet< Serializer* > find(const QString& extension_)
                {
                    QSet< Serializer* > matches;
                    QSet< FileFormat* > formats = FileFormat::getForExtension(extension_);
                    QSetIterator< FileFormat* > format(formats);
                    while (format.hasNext())
                    {
                        FileFormat* next = format.next();
                        Serializer* serializer = find(next);
                        if (serializer)
                        {
                            matches += serializer;
                        }
                    }
                    return matches;
                }
            static bool insert(FileFormat* format_, Serializer* serializer_)
                {
                    registry& reg = get();
                    if (serializer_ && format_ && !reg.contains(format_))
                    {
                        reg[format_] = serializer_;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            static QList< Serializer* > serializers()
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

    Serializer::Context::Context(const Serializer* serializer_)
        : _serializer(serializer_), _errorCode(None)
    {}

    const Serializer* Serializer::Context::serializer() const
    {
        return this->_serializer;
    }

    /** Return error code. */
    Serializer::ErrorCode Serializer::Context::errorCode() const
    {
        return this->_errorCode;
    }

    /** Return message. */
    QString Serializer::Context::message() const
    {
        return this->_message;
    }

    /** Set error code. */
    void Serializer::Context::setErrorCode(Serializer::ErrorCode errorCode_)
    {
        this->_errorCode = errorCode_;
    }

    /** Set message. */
    void Serializer::Context::setMessage(const QString& message_)
    {
        this->_message = message_;
    }



    /** Parse! */
    Serializer::Context Serializer::serialize(QIODevice& stream_, Node* node_) const
    {
        Serializer::Context ctx(this);
        this->serialize(ctx, stream_, node_);
        return ctx;
    }

    /** Default format list */
    QSet< FileFormat* > Serializer::formats() const
    {
        QSet< FileFormat* > formats;
        return formats;
    }

    /** Return format category */
    FormatCategory Serializer::formatCategory() const
    {
        return _formatCategory;
    }

    void Serializer::registerFormat(FileFormat* format_, Serializer* serializer_)
    {
        registry::insert(format_, serializer_);
    }

    /**
     *  \brief Get Serializer for a given FileFormat.
     */
    Serializer* Serializer::get(FileFormat* format_)
    {
        return registry::find(format_);
    }

    /**
     *  \brief Get Serializer for a given file extension.
     */
    QSet< Serializer* > Serializer::get(const QString& extension_)
    {
        QSet< Serializer* > serializers;
        QList< FileFormat* > formats = registry::formats();
        for (int i = 0; i < formats.size(); ++i)
        {
            FileFormat* format = formats.at(i);
            if (format->contains(extension_))
            {
                serializers.insert(registry::find(format));
            }
        }
        return serializers;
    }

    /**
     *  \brief Get all available Serializers.
     */
    QSet< Serializer* > Serializer::all()
    {
        QSet< Serializer* > all;
        QList< Serializer* > serializers = registry::serializers();
        for (int i = 0; i < serializers.size(); ++i)
        {
            all.insert(serializers.at(i));
        }
        return all;
    }

    /** Serialize stream using given file format. */
    Serializer::Context serialize(QIODevice& stream_, Node* node_, FileFormat* fileFormat_)
    {
        Serializer* serializer = Serializer::get(fileFormat_);
        if (serializer)
        {
            return serializer->serialize(stream_, node_);
        }
        else
        {
            return Serializer::Context(0);
        }
    }

    /** Load file using given file format. */
    Serializer::Context save(const QString& fileName_, Node* node_, FileFormat* fileFormat_)
    {
        QFile file(fileName_);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        if (fileFormat_ == 0)
        {
            QSet< FileFormat* > formats = FileFormat::getForExtension(fileName_.section(".", -1, -1));
            if (formats.count() != 1)
            {
                // FIXME cannot distringuish file format from extension
                return Serializer::Context(0);
            }
            else
            {
                fileFormat_ = *formats.begin();
            }
        }
        return serialize(file, node_, fileFormat_);
    }

} // namespace Utopia

UTOPIA_DEFINE_EXTENSION_CLASS(Utopia::Serializer)
