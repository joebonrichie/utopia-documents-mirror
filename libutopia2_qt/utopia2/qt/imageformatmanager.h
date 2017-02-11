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

#ifndef Utopia_QT_IMAGEFORMATMANAGER_H
#define Utopia_QT_IMAGEFORMATMANAGER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QStringList>

class QMimeData;
class QUrl;

namespace Utopia
{

    class ImageFormatManagerPrivate;
    class ImageFormatManager : public QObject
    {
        Q_OBJECT

    public:
        typedef enum
        {
            Read,
            Write
        } FileMode;

        ~ImageFormatManager();

        static boost::shared_ptr< ImageFormatManager > instance();

        QString defaultExportFormat() const;
        int exportQuality() const;
        void setDefaultExportFormat(const QString & format);
        void setExportQuality(int quality);

        QStringList extensionsForFormat(const QString & format) const;
        QString formatForExtension(const QString & extension, FileMode mode) const;
        QString mimeDataSupported(const QMimeData * mimeData);
        QStringList supportedExportFormats() const;
        bool urlSupported(const QUrl & url);

        static QPixmap loadImageFile(QWidget * parent, const QString & title);
        static bool saveImageFile(QWidget * parent, const QString & title, const QPixmap & pixmap, const QString & baseName = QString());

    signals:
        void exportQualityChanged(int quality);
        void defaultExportFormatChanged(const QString & format);

    protected:
        ImageFormatManager();

        boost::scoped_ptr< ImageFormatManagerPrivate > d;
    };

}

#endif // Utopia_QT_IMAGEFORMATMANAGER_H
