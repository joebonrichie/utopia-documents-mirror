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

#ifndef Utopia_FILEFIXERDIALOG_H
#define Utopia_FILEFIXERDIALOG_H

#include <utopia2/qt/config.h>
#include <utopia2/enums.h>
#include <utopia2/parser.h>

#include <QDialog>
#include <QModelIndex>

class QTemporaryFile;

namespace Ui { class FileFixerDialog; }

namespace Utopia
{

    class LIBUTOPIA_QT_API FileFixerDialog : public QDialog
    {
        Q_OBJECT

    public:
        typedef enum {
            NoExport        = 0x0,
            AskExport       = 0x1,
            ForceExport     = 0x2,
            UseSameFileName = 0x4
        } ExportPolicyFlag;
        Q_DECLARE_FLAGS(ExportPolicy, ExportPolicyFlag);

        // Constructor
        FileFixerDialog(Parser::Context ctx_, const QString& fileName_, FileFormat* fileFormat_, ExportPolicy exportPolicy = AskExport);
        // Destructor
        ~FileFixerDialog();

        // Accessor
        Parser::Context ctx() const;
        QString exportFileName() const;

    protected slots:
        // Retry
        void retry();
        // Goto Error/Warning
        void goTo(const QModelIndex& index_);

    private:
        // UI
        Ui::FileFixerDialog* _ui;
        // Temporary File
        QTemporaryFile* _tempFile;
        // Parser Context
        Parser::Context _ctx;
        // File Format
        FileFormat* _fileFormat;
        // Original File Name
        QString _fileName;
        // Exported File Name
        QString _exportFileName;
        // Policy regarding exporting
        ExportPolicy _exportPolicy;

    }; // class FileFixerDialog

} // namespace Utopia

Q_DECLARE_OPERATORS_FOR_FLAGS(Utopia::FileFixerDialog::ExportPolicy);

#endif // Utopia_FILEFIXERDIALOG_H
