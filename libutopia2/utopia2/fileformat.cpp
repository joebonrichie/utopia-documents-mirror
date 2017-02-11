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

#include <utopia2/fileformat.h>

#include <QMap>

namespace Utopia
{

    /**
     *  \brief Query this FileFormat's name.
     */
    QString FileFormat::name() const
    {
        return _name;
    }

    /**
     *  \brief Serialise this format's extension list.
     */
    QString FileFormat::masks(const QString& delimiter_) const
    {
        QString masks;
        bool first = true;

        QSetIterator< QString > exts(*this);
        while (exts.hasNext())
        {
            QString ext = exts.next();

            // Pad the extensions with the delimiter
            if (first)
            {
                first = false;
            }
            else
            {
                masks += delimiter_;
            }
            masks += QString("*.%1").arg(ext);
        }

        return masks;
    }

    /**
     *  \brief Get format categories mask.
     */
    int FileFormat::formatCategories() const
    {
        return this->_formatCategories;
    }

    namespace
    {

        class registry : protected QMap< QString, FileFormat* >
        {
            typedef QMap< QString, FileFormat* > _Base;
            typedef QMapIterator< QString, FileFormat* > _BaseIterator;

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
            static FileFormat* find(const QString& name_)
                {
                    if (get().contains(name_))
                    {
                        return get()[name_];
                    }
                    else
                    {
                        return 0;
                    }
                }
            static bool insert(FileFormat* format_)
                {
                    if (format_ && !get().contains(format_->name()))
                    {
                        get()[format_->name()] = format_;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            static QList< FileFormat* > formats()
                {
                    return get().values();
                }

        private:
            // Constructor
            registry()
                : _Base()
                {}

            // Static registry get
            static registry& get()
                {
                    static registry reg;
                    return reg;
                }

        }; // class registry

    } // Anonymous namespace

    /**
     *  \brief Static resgistration function for file formats.
     */
    FileFormat* FileFormat::create(const QString& name_, int formatCategories_)
    {
        FileFormat* format = registry::find(name_);
        if (format)
        {
            format->_formatCategories |= formatCategories_;
        }
        else
        {
            format = new FileFormat(name_, formatCategories_);
            if (!registry::insert(format))
            {
                delete format;
                format = 0;
            }
        }
        return format;
    }

    /**
     *  \brief Static get function for file formats.
     */
    FileFormat* FileFormat::get(const QString& name_)
    {
        return registry::find(name_);
    }

    /**
     *  \brief Static get function for file formats.
     */
    QSet< FileFormat* > FileFormat::get(int formatCategories_)
    {
        QSet< FileFormat* > matches;
        QList< FileFormat* > formats = registry::formats();
        for (int i = 0; i < formats.size(); ++i)
        {
            FileFormat* format = formats.at(i);
            if (format->formatCategories() & formatCategories_)
            {
                matches += format;
            }
        }
        return matches;
    }

    /**
     *  \brief Static get function for formats matching a given extension.
     */
    QSet< FileFormat* > FileFormat::getForExtension(const QString& extension_, int formatCategories_)
    {
        QSet< FileFormat* > matches;
        QList< FileFormat* > formats = registry::formats();
        for (int i = 0; i < formats.size(); ++i)
        {
            FileFormat* format = formats.at(i);
            if (format->contains(extension_) &&
                (format->formatCategories() & formatCategories_))
            {
                matches += format;
            }
        }
        return matches;
    }

    /**
     *  \brief Static query function.
     */
    QSet< FileFormat* > FileFormat::all()
    {
        QSet< FileFormat* > all;
        QList< FileFormat* > formats = registry::formats();
        for (int i = 0; i < formats.size(); ++i)
        {
            all += formats.at(i);
        }
        return all;
    }

    /**
     *  \brief Constructor of FileFormat object.
     */
    FileFormat::FileFormat(const QString& name_, int formatCategories_)
        : QSet< QString >(), _name(name_), _formatCategories(formatCategories_)
    {}


} // namespace Utopia
