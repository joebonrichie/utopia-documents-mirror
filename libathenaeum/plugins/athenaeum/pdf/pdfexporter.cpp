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

#include "pdfexporter.h"

#include <papyro/abstractbibliography.h>
#include <crackle/PDFDocument.h>

#include <QFile>
#include <QUrl>

#include <QDebug>

using namespace Athenaeum;

// Private method
bool PDFExporter::doExport(const QModelIndex & index, const QString & filename)
{
    QString sourceFileName = index.data(Citation::ObjectFileRole).toUrl().toLocalFile();
    qDebug() << "source filename = " << sourceFileName;
    QFile sourceFile(sourceFileName);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open source file";
        return false;
    }

    qDebug() << "target filename = " << filename;
    /*
    // UPDATED The dialog will ask about overwriting, so probably not necc.
    // TODO check for existence
    QString newFilename = filename;
    int i = 1;
    while (QFile::exists(newFilename))
    {
        newFilename = filename;
        newFilename.chop(4);
        newFilename = QString("%1 (%2).pdf").arg(newFilename).arg(i++);
    }
    QFile targetFile(newFilename);
    */
    QFile targetFile(filename);
    if (!targetFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open target file";
        return false;
    }
    targetFile.write(sourceFile.readAll());
    targetFile.close();
    sourceFile.close();

    return true;
}

// Will return false if at least one file failed
bool PDFExporter::doExport(const QModelIndexList & indexList, const QString & filename)
{
    bool result = true;

    // TODO Make sure the filename is just a path
    QString baseDirectory = filename;

    if (indexList.size() == 1) {
        result = doExport(indexList.at(0), baseDirectory);
    } else {
        for (int i = 0; result && i < indexList.size(); ++i) {
            if (!baseDirectory.endsWith("/")) {
                baseDirectory.append("/");
            }
            QString filename = indexList.at(i).data(Citation::TitleRole).toString().left(64);
            if (filename.isEmpty()) {
                filename = "Untitled Document";
            }
            result = doExport(indexList.at(i), baseDirectory + filename + ".pdf");
        }
    }
    return result;
}

QStringList PDFExporter::extensions() const
{
    QStringList exts;
    exts << "pdf";
    return exts;
}

QString PDFExporter::name() const
{
    return "PDF";
}

bool PDFExporter::multipleFiles() const
{
    return true;
}

