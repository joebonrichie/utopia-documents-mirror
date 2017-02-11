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

#include <utopia2/qt/imageformatmanager.h>

#include <boost/weak_ptr.hpp>

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMap>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QUrl>

namespace Utopia
{

    //// ImageFormatManagerPrivate ////////////////////////////////////////////////////////////////

    class ImageFormatManagerPrivate
    {
    public:
        ImageFormatManagerPrivate(ImageFormatManager * manager)
            : manager(manager), exportQuality(100)
            {
                // Supported mime types (format -> (mimes, extensions))
                QMap< QString, QPair< QStringList, QStringList > > formats;
                formats["bmp"].first << "image/bmp";
                formats["bmp"].second << "bmp";
                formats["gif"].first << "image/gif";
                formats["gif"].second << "gif";
                formats["ico"].first << "image/x-icon";
                formats["ico"].second << "ico";
                formats["svg"].first << "image/svg+xml";
                formats["svg"].second << "svg";
                formats["tiff"].first << "image/tiff";
                formats["tiff"].second << "tiff";
                formats["tiff"].second << "tif";
                //formats["tif"] = formats["tiff"];
                formats["jpeg"].first << "image/jpg";
                formats["jpeg"].first << "image/jpeg";
                formats["jpeg"].second << "jpg";
                formats["jpeg"].second << "jpeg";
                formats["jpeg"].second << "jpe";
                //formats["jpg"] = formats["jpeg"];
                formats["mng"].first << "video/x-mng";
                formats["mng"].second << "mng";
                formats["png"].first << "image/png";
                formats["png"].second << "png";
                formats["pbm"].first << "image/x-portable-bitmap";
                formats["pbm"].second << "pbm";
                formats["pgm"].first << "image/x-portable-graymap";
                formats["pgm"].second << "pgm";
                formats["ppm"].first << "image/x-portable-pixmap";
                formats["ppm"].second << "ppm";
                formats["xbm"].first << "image/x-xbitmap";
                formats["xbm"].second << "xbm";
                formats["xpm"].first << "image/x-xpixmap";
                formats["xpm"].second << "xpm";
                QListIterator< QByteArray > supported(QImageReader::supportedImageFormats());
                while (supported.hasNext())
                {
                    QString format(supported.next());
                    if (formats.contains(format))
                    {
                        supportedFormats[ImageFormatManager::Read][format] = formats[format];
                        supportedMimeTypes[ImageFormatManager::Read] += formats[format].first;
                        supportedExtensions[ImageFormatManager::Read] += formats[format].second;
                    }
                }
                supported = QImageWriter::supportedImageFormats();
                while (supported.hasNext())
                {
                    QString format(supported.next());
                    if (formats.contains(format))
                    {
                        supportedFormats[ImageFormatManager::Write][format] = formats[format];
                        supportedMimeTypes[ImageFormatManager::Write] += formats[format].first;
                        supportedExtensions[ImageFormatManager::Write] += formats[format].second;
                    }
                }

                supportedExtensions[ImageFormatManager::Read].removeDuplicates();
                supportedExtensions[ImageFormatManager::Write].removeDuplicates();
                supportedMimeTypes[ImageFormatManager::Read].removeDuplicates();
                supportedMimeTypes[ImageFormatManager::Write].removeDuplicates();

                // Get settings
                QSettings settings;
                settings.beginGroup("/Image Formats");
                previousFileOpenPath = settings.value("/previousFileOpenPath").toString();
                previousFileSavePath = settings.value("/previousFileSavePath").toString();
                defaultExportFormat = settings.value("/defaultExportFormat", "jpeg").toString();
                exportQuality = qBound(0, settings.value("/exportQuality", 100).toInt(), 100);
                settings.endGroup();
            }

        ~ImageFormatManagerPrivate()
            {
                // Save settings
                QSettings settings;
                settings.beginGroup("/Image Formats");
                settings.setValue("/previousFileOpenPath", previousFileOpenPath);
                settings.setValue("/previousFileSavePath", previousFileSavePath);
                settings.setValue("/defaultExportFormat", defaultExportFormat);
                settings.setValue("/exportQuality", exportQuality);
                settings.endGroup();
            }

        ImageFormatManager * manager;

        QString previousFileOpenPath;
        QString previousFileSavePath;
        QString defaultExportFormat;
        int exportQuality;

        QMap< ImageFormatManager::FileMode, QMap< QString, QPair< QStringList, QStringList > > > supportedFormats;
        QMap< ImageFormatManager::FileMode, QStringList > supportedMimeTypes;
        QMap< ImageFormatManager::FileMode, QStringList > supportedExtensions;
    };


    //// ImageFormatManager ///////////////////////////////////////////////////////////////////////

    ImageFormatManager::ImageFormatManager()
        : QObject(), d(new ImageFormatManagerPrivate(this))
    {}

    ImageFormatManager::~ImageFormatManager()
    {}

    QString ImageFormatManager::defaultExportFormat() const
    {
        return d->defaultExportFormat;
    }

    int ImageFormatManager::exportQuality() const
    {
        return d->exportQuality;
    }

    QStringList ImageFormatManager::extensionsForFormat(const QString & format) const
    {
        QStringList extensions;
        if (d->supportedFormats[ImageFormatManager::Read].contains(format))
        {
            extensions += d->supportedFormats[ImageFormatManager::Read][format].second;
        }
        if (d->supportedFormats[Write].contains(format))
        {
            extensions += d->supportedFormats[Write][format].second;
        }
        extensions.removeDuplicates();
        return extensions;
    }

    QString ImageFormatManager::formatForExtension(const QString & extension, FileMode mode) const
    {
        QString found;
        QListIterator< QByteArray > supported(mode == ImageFormatManager::Read ? QImageReader::supportedImageFormats() : QImageWriter::supportedImageFormats());
        while (supported.hasNext())
        {
            QString format(supported.next());
            if (d->supportedFormats[mode].contains(format) && d->supportedFormats[mode][format].second.contains(extension))
            {
                found = format;
                break;
            }
        }
        return found;
    }

    boost::shared_ptr< ImageFormatManager > ImageFormatManager::instance()
    {
        static boost::weak_ptr< ImageFormatManager > singleton;
        boost::shared_ptr< ImageFormatManager > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< ImageFormatManager >(new ImageFormatManager());
            singleton = shared;
        }
        return shared;
    }

    QPixmap ImageFormatManager::loadImageFile(QWidget * parent, const QString & title)
    {
        boost::shared_ptr< Utopia::ImageFormatManager > formatManager(Utopia::ImageFormatManager::instance());
        QPixmap pixmap;
        bool success = false;
        QString filter = QString("Image Files (*.%1);; All Files (*.*)").arg(formatManager->d->supportedExtensions[Read].join(" *."));
        while (!success)
        {
            QString fileName(QFileDialog::getOpenFileName(parent, title, formatManager->d->previousFileOpenPath, filter));
            if (!fileName.isEmpty())
            {
                formatManager->d->previousFileOpenPath = QFileInfo(fileName).absolutePath();
                pixmap = QPixmap(fileName);
                if (pixmap.isNull())
                {
                    QString message("The chosen image file's format was not recognised. "
                                    "Would you like to try choosing a different image file?");
                    success = QMessageBox::critical(parent, "Error Opening File", message, QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry) == QMessageBox::Cancel;
                }
                else
                {
                    success = true;
                }
            }
            else
            {
                success = true;
            }
        }
        return pixmap;
    }

    QString ImageFormatManager::mimeDataSupported(const QMimeData * mimeData)
    {
        // If URLs, only accept one, and only if it has an acceptable extension
        QStringList mimes(d->supportedMimeTypes[ImageFormatManager::Read]);
        QStringListIterator supported(mimes);
        while (supported.hasNext())
        {
            QString mime(supported.next());
            if (mimeData->hasFormat(mime))
            {
                return mime;
            }
        }

        return QString();
    }

    bool ImageFormatManager::saveImageFile(QWidget * parent, const QString & title, const QPixmap & pixmap, const QString & baseName)
    {
        boost::shared_ptr< Utopia::ImageFormatManager > formatManager(Utopia::ImageFormatManager::instance());
        bool success = false;
        QString filter;
        QString chosenFilter;
        QStringListIterator formats(formatManager->supportedExportFormats());
        while (formats.hasNext())
        {
            QString format(formats.next());
            if (!filter.isEmpty()) { filter += ";;"; }
            QString filterLine = QString("%1 (*.%2)").arg(format.toUpper()).arg(formatManager->extensionsForFormat(format).first());
            if (format == formatManager->defaultExportFormat())
            {
                chosenFilter = filterLine;
            }
            filter += filterLine;
        }
        QString suggestedBaseName(baseName);
        if (suggestedBaseName.isEmpty())
        {
            suggestedBaseName = "Untitled";
        }
        QString fileName(QFileDialog::getSaveFileName(parent, "Save File...", QDir(formatManager->d->previousFileSavePath).filePath(suggestedBaseName), filter, &chosenFilter));
        if (!fileName.isEmpty() && !chosenFilter.isEmpty())
        {
            formatManager->d->previousFileSavePath = QFileInfo(fileName).absolutePath();

            while (!(success = pixmap.save(fileName, chosenFilter.section(" ", 0, 0).toUtf8().constData(), formatManager->exportQuality())))
            {
                QString message("Unable to save image file. "
                                "Would you like to try choosing a different location to save?");
                if (QMessageBox::critical(parent, "Error Saving File", message, QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry) == QMessageBox::Cancel)
                {
                    break;
                }
            }
        }

        return success;
    }

    void ImageFormatManager::setDefaultExportFormat(const QString & format)
    {
        if (d->defaultExportFormat != format)
        {
            d->defaultExportFormat = format;
            emit defaultExportFormatChanged(format);
        }
    }

    void ImageFormatManager::setExportQuality(int quality)
    {
        quality = qBound(0, quality, 100);
        if (d->exportQuality != quality)
        {
            d->exportQuality = quality;
            emit exportQualityChanged(quality);
        }
    }

    QStringList ImageFormatManager::supportedExportFormats() const
    {
        return d->supportedFormats[Write].keys();
    }

    bool ImageFormatManager::urlSupported(const QUrl & url)
    {
        if (url.scheme() == "file")
        {
            QFileInfo fileInfo(url.toLocalFile());
            if (d->supportedExtensions[ImageFormatManager::Read].contains(fileInfo.suffix()))
            {
                return true;
            }
        }

        return false;
    }

}
