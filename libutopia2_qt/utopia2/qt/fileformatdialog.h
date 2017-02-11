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

#ifndef Utopia_FILEFORMATDIALOG_H
#define Utopia_FILEFORMATDIALOG_H

#include <utopia2/qt/config.h>
#include <utopia2/enums.h>
#include <utopia2/fileformat.h>

#include <QDialog>

class QIODevice;

namespace Ui { class FileFormatDialog; }

namespace Utopia
{

    class LIBUTOPIA_QT_API FileFormatDialog : public QDialog
    {
        Q_OBJECT

    public:
        // Constructor
        FileFormatDialog(const QString& filename_, int formatCategories_ = AnyFormat);
        FileFormatDialog(QIODevice * io_, int formatCategories_ = AnyFormat);
        // Destructor
        ~FileFormatDialog();

        // Accessor
        FileFormat* selectedFileFormat() const;

    private:
        // UI
        Ui::FileFormatDialog* _ui;

        // Private
        void init(const QSet< FileFormat* > & formats);
        void populate(QIODevice * io);

    }; // class FileFormatDialog

} // namespace Utopia

#endif // Utopia_FILEFORMATDIALOG_H
