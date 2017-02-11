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

#ifndef Utopia_QT_FILEDIALOG
#define Utopia_QT_FILEDIALOG

#include <utopia2/qt/config.h>
#include <utopia2/enums.h>
#include <utopia2/fileformat.h>

#include <QFileDialog>
#include <QString>

class QWidget;

namespace Utopia
{

    // Functions to get a filename for opening
    LIBUTOPIA_QT_EXPORT QString getOpenFileName(FileFormat* fileFormat_,
                                                QWidget* parent_ = 0,
                                                const QString& caption_ = QString(),
                                                const QString& dir_ = QString(),
                                                QFileDialog::Options options_ = 0);
    LIBUTOPIA_QT_EXPORT QPair< QString, FileFormat* > getOpenFileName(QWidget* parent_ = 0,
                                                                      const QString& caption_ = QString(),
                                                                      const QString& dir_ = QString(),
                                                                      int formatCategories_ = AnyFormat,
                                                                      QFileDialog::Options options_ = 0);

    // function to get a filename for exporting
    LIBUTOPIA_QT_EXPORT QString getExportFileName(FileFormat* fileFormat_,
                                                  QWidget* parent_ = 0,
                                                  const QString& caption_ = QString(),
                                                  const QString& dir_ = QString(),
                                                  QFileDialog::Options options_ = 0);
    LIBUTOPIA_QT_EXPORT QPair< QString, FileFormat* > getExportFileName(QWidget* parent_ = 0,
                                                                        const QString& caption_ = QString(),
                                                                        const QString& dir_ = QString(),
                                                                        int formatCategories_ = AnyFormat,
                                                                        QFileDialog::Options options_ = 0);

    // function to get a filename for saving
    LIBUTOPIA_QT_EXPORT QString getSaveFileName(QWidget* parent_ = 0,
                                                const QString& caption_ = QString(),
                                                const QString& dir_ = QString(),
                                                QFileDialog::Options options_ = 0);
}

#endif // Utopia_QT_FILEDIALOG
