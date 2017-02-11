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

#ifndef Utopia_FILEFORMAT_H
#define Utopia_FILEFORMAT_H

#include <utopia2/config.h>
#include <utopia2/enums.h>
#include <utopia2/fileformat.h>
#include <utopia2/library.h>

#include <QSet>
#include <QString>

namespace Utopia
{

    class LIBUTOPIA_API FileFormat : public QSet< QString >
    {
    public:
        // Get format name
        QString name() const;
        // Get format categories
        int formatCategories() const;

        // Get extensions as a delimiter-seperated list of file masks
        QString masks(const QString& delimiter_ = QString(" ")) const;

        // Static methods
        static FileFormat* create(const QString& name_, int formatCategories_ = AnyFormat);
        static FileFormat* get(const QString& name_);
        static QSet< FileFormat* > get(int formatCategories_);
        static QSet< FileFormat* > getForExtension(const QString& extension_, int formatCategories_ = AnyFormat);
        static QSet< FileFormat* > all();

    private:
        // Constructor
        FileFormat(const QString& name_, int formatCategories_);

        // Format Name
        QString _name;
        // Format Categories
        int _formatCategories;

    }; /* class FileFormat */

} /* namespace Utopia */

#endif // Utopia_FILEFORMAT_H
