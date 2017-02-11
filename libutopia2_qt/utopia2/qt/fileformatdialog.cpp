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

#include <utopia2/qt/fileformatdialog.h>

#include <QFile>
#include <QString>
#include "utopia2/qt/ui_fileformatdialog.h"

#include <QTextStream>

namespace Utopia
{

    FileFormatDialog::FileFormatDialog(const QString& filename_, int formatCategories_)
    {
        // Ascertain from the filename the potentially matching fileformats
        QSet< FileFormat* > formats;
        if (filename_.indexOf('.') > 0)
        {
            formats = FileFormat::getForExtension(filename_.section('.', -1), formatCategories_);
        }
        if (formats.size() == 0)
        {
            formats = FileFormat::get(formatCategories_);
        }

        this->init(formats);

        // Populate preview pane if possible
        QFile file(filename_);
        file.open(QIODevice::ReadOnly);
        this->populate(&file);
    }

    FileFormatDialog::FileFormatDialog(QIODevice * io_, int formatCategories_)
    {
        // Ascertain from the filename the potentially matching fileformats
        QSet< FileFormat* > formats = FileFormat::get(formatCategories_);

        this->init(formats);
        this->populate(io_);
    }

    void FileFormatDialog::init(const QSet< FileFormat* > & formats)
    {
        // Set up user interface
        this->_ui = new Ui::FileFormatDialog;
        this->_ui->setupUi(this);

        QMap< QString, FileFormat * > formatNames;
        // Populate combo box
        QSetIterator< FileFormat* > i(formats);
        while (i.hasNext())
        {
            FileFormat * format = i.next();
            formatNames[format->name()] = format;
        }

        // Populate combo box
        QMapIterator< QString, FileFormat * > j(formatNames);
        while (j.hasNext())
        {
            j.next();
            this->_ui->formats->addItem(j.key());
        }
    }

    void FileFormatDialog::populate(QIODevice * io)
    {
        QTextStream fileStream(io);
        QString binaryCheckString = fileStream.read(1024);
        bool ascii = true;
        for (int i = 0; ascii && i < binaryCheckString.size(); ++i)
        {
            ascii = ascii && binaryCheckString.at(i).toLatin1();
        }

        if (ascii)
        {
            this->_ui->labelBinary->hide();
            this->_ui->preview->show();

            // Display first 100 lines
            fileStream.seek(0);
            for (int i = 0; i < 100; ++i)
            {
                if (fileStream.atEnd())
                {
                    break;
                }

                this->_ui->preview->append(fileStream.readLine());
            }
            this->_ui->preview->moveCursor(QTextCursor::Start);
        }
        else
        {
            this->_ui->labelBinary->show();
            this->_ui->preview->hide();
        }
    }

    FileFormat* FileFormatDialog::selectedFileFormat() const
    {
        return FileFormat::get(this->_ui->formats->currentText());
    }

    FileFormatDialog::~FileFormatDialog()
    {
        delete this->_ui;
    }

} // namespace Utopia
