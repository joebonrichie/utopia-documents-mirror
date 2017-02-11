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

#include <utopia2/qt/filedialog.h>
#include <utopia2/qt/fileformatdialog.h>
#include <utopia2/fileformat.h>

#include <QFileInfo>
#include <QMap>
#include <QPair>
#include <QSettings>
#include <QStringList>

#include <QtDebug>

namespace Utopia
{

    namespace
    {
        QString lastVisitedDirectoryPath(const QString & category)
        {
            // Get settings
            QSettings settings;
            settings.beginGroup("/File Dialogs");
            return settings.value("/lastVisitedDirectoryPath/" + category).toString();
        }

        void setLastVisitedDirectoryPath(const QString & category, const QString & path)
        {
            // Save settings
            QSettings settings;
            settings.beginGroup("/File Dialogs");
            settings.setValue("/lastVisitedDirectoryPath/" + category, path);
            settings.endGroup();
        }
    }

    /**
     *  \brief Get a filename for opening.
     */
    QString getOpenFileName(FileFormat* fileFormat_,
                            QWidget* parent_,
                            const QString& caption_,
                            const QString& dir_,
                            QFileDialog::Options options_)
    {
        QString filter;
        bool extensionFirst = true;
        filter = QString("%1 (").arg(fileFormat_->name());
        QSetIterator< QString > exts(*fileFormat_);
        while (exts.hasNext())
        {
            QString extension = QString("*.%1").arg(exts.next());

            if (extensionFirst)
            {
                extensionFirst = false;
            }
            else
            {
                // Pad extensions with a space
                filter += " ";
            }
            filter += extension;
        }
        filter += ")";

        QString newDir = dir_;
        if (newDir.isEmpty() && !lastVisitedDirectoryPath("Open").isEmpty())
        {
            newDir = lastVisitedDirectoryPath("Open");
        }

        // Delegate to QFileDialog::getOpenFileName()
        QString returnedPath = QFileDialog::getOpenFileName(parent_, caption_, newDir, filter, 0, options_);

        if (!returnedPath.isEmpty())
        {
            setLastVisitedDirectoryPath("Open", QFileInfo(returnedPath).dir().path());
        }

        return returnedPath;
    }

    /**
     *  \brief Get a filename for opening.
     */
    QPair< QString, FileFormat* > getOpenFileName(QWidget* parent_,
                                                  const QString& caption_,
                                                  const QString& dir_,
                                                  int formatCategories_,
                                                  QFileDialog::Options options_)
    {
        QSet< FileFormat* > registeredFormats = FileFormat::get(formatCategories_);
        QMap< QString, QSet< QString > > formats;
        QSetIterator< FileFormat* > registeredFormat(registeredFormats);
        while (registeredFormat.hasNext())
        {
            FileFormat* format = registeredFormat.next();
            formats[format->name()] = *format;
        }

        // Compile filter from parsers
        QString filter;
        QString all;
        bool allFirst = true;
        QMapIterator< QString, QSet< QString > > parser(formats);
        while (parser.hasNext()) {
            parser.next();
            QString parserName = parser.key();
            QSet< QString > extensions = parser.value();
            bool extensionFirst = true;
            filter += QString("%1 (").arg(parserName);
            QSetIterator< QString > exts(extensions);
            while (exts.hasNext())
            {
                QString extension = QString("*.%1").arg(exts.next());

                if (extensionFirst)
                {
                    extensionFirst = false;
                }
                else
                {
                    // Pad extensions with a space
                    filter += " ";
                }
                filter += extension;

                if (allFirst)
                {
                    allFirst = false;
                }
                else
                {
                    // Pad extensions with a space
                    all += " ";
                }
                all += extension;
            }
            filter += ")\n";
        }
        all = QString("All known file types (%1)").arg(all);
        filter = filter + all + QString("\nAll files (*)");

        QString newDir = dir_;
        if (newDir.isEmpty() && !lastVisitedDirectoryPath("Open").isEmpty())
        {
            newDir = lastVisitedDirectoryPath("Open");
        }

        // Delegate to QFileDialog::getOpenFileName()
        QString filename = QFileDialog::getOpenFileName(parent_, caption_, newDir, filter, &all, options_);
        FileFormat* selectedFormat = 0;

        if (!filename.isEmpty())
        {
            setLastVisitedDirectoryPath("Open", QFileInfo(filename).dir().path());

            // Figure out format...
            QString fileFormatName = all.section(" (", -2, -2);
            selectedFormat = FileFormat::get(fileFormatName);
            QString suffix = QFileInfo(filename).suffix();
            if (selectedFormat == 0 && !suffix.isEmpty())
            {
                QSet< FileFormat* > matchingFormats = FileFormat::getForExtension(suffix, formatCategories_);
                if (matchingFormats.size() == 1)
                {
                    selectedFormat = *matchingFormats.begin();
                }
            }
            if (selectedFormat == 0)
            {
                FileFormatDialog dialog(filename, formatCategories_);
                if (dialog.exec() == QDialog::Accepted)
                {
                    selectedFormat = dialog.selectedFileFormat();
                }
                else
                {
                    filename = "";
                }
            }
        }

        return qMakePair(filename, selectedFormat);
    }

    /**
     *  \brief Get a filename for exporting.
     */
    QString getExportFileName(FileFormat* fileFormat_,
                              QWidget* parent_,
                              const QString& caption_,
                              const QString& dir_,
                              QFileDialog::Options options_)
    {
        QString filter;
        bool extensionFirst = true;
        filter = QString("%1 (").arg(fileFormat_->name());
        QSetIterator< QString > exts(*fileFormat_);
        while (exts.hasNext())
        {
            QString extension = QString("*.%1").arg(exts.next());

            if (extensionFirst)
            {
                extensionFirst = false;
            }
            else
            {
                // Pad extensions with a space
                filter += " ";
            }
            filter += extension;
        }
        filter += ")";

        // Delegate to QFileDialog::getSaveFileName()
        QString filename = QFileDialog::getSaveFileName(parent_, caption_, dir_, filter, 0, options_);

        if (!filename.isEmpty())
        {
            QString suffix = QFileInfo(filename).suffix().toLower();

            // Provide suffix if not present
            bool found = false;
            if (!suffix.isEmpty())
            {
                QSetIterator< QString > exts(*fileFormat_);
                while (exts.hasNext())
                {
                    if (suffix == exts.next().toLower())
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found && !fileFormat_->isEmpty())
            {
                filename += QString(".%1").arg(*fileFormat_->begin());
            }
        }

        return filename;
    }

    /**
     *  \brief Get a filename for exporting.
     */
    QPair< QString, FileFormat* > getExportFileName(QWidget* parent_,
                                                    const QString& caption_,
                                                    const QString& dir_,
                                                    int formatCategories_,
                                                    QFileDialog::Options options_)
    {
        // Temporary map
        QMap< QString, QSet< QString > > formats;
        QSet< FileFormat* > registeredFormats = FileFormat::get(formatCategories_);
        QSetIterator< FileFormat* > registeredFormat(registeredFormats);
        while (registeredFormat.hasNext())
        {
            FileFormat* format = registeredFormat.next();
            formats[format->name()] = *format;
        }

        // Compile filter from serializers
        QString filter;
        QString utopia;
        bool allFirst = true;
        QMapIterator< QString, QSet< QString > > serializer(formats);
        while (serializer.hasNext()) {
            serializer.next();
            QString serializerName = serializer.key();
            QSet< QString > extensions = serializer.value();
            bool extensionFirst = true;
            QString filterLine = QString("%1 (").arg(serializerName);
            QSetIterator< QString > exts(extensions);
            while (exts.hasNext())
            {
                QString extension = QString("*.%1").arg(exts.next());

                if (extensionFirst)
                {
                    extensionFirst = false;
                }
                else
                {
                    // Pad extensions with a space
                    filterLine += " ";
                }
                filterLine += extension;

                if (allFirst)
                {
                    allFirst = false;
                }
            }
            filterLine += ")\n";
            if (serializerName == "Utopia")
            {
                utopia = filterLine;
            }
            filter += filterLine;
        }

        // Delegate to QFileDialog::getSaveFileName()
        QString filename = QFileDialog::getSaveFileName(parent_, caption_, dir_, filter, &utopia, options_);
        FileFormat* format = FileFormat::get(utopia.section(" (", -2, -2));

        if (!filename.isEmpty())
        {
            QString suffix = QFileInfo(filename).suffix().toLower();

            // Provide suffix if not present
            bool found = false;
            if (!suffix.isEmpty())
            {
                QSetIterator< QString > exts(*format);
                while (exts.hasNext())
                {
                    if (suffix == exts.next().toLower())
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found && !format->isEmpty())
            {
                filename += QString(".%1").arg(*format->begin());
            }
        }

        // Figure out format...
        return qMakePair(filename, format);
    }

    /**
     *  \brief Get a filename for saving.
     */
    QString getSaveFileName(QWidget* parent_,
                            const QString& caption_,
                            const QString& dir_,
                            QFileDialog::Options options_)
    {
        FileFormat* utopia = FileFormat::get("UTOPIA");
        return getExportFileName(utopia, parent_, caption_, dir_, options_);
    }

} // namespace Utopia
