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

#include "utopia2/qt/ui_filefixerdialog.h"

#include <utopia2/qt/filedialog.h>
#include <utopia2/qt/filefixerdialog.h>
#include <utopia2/qt/parsercontextmodel.h>

#include <QTemporaryFile>
#include <QMessageBox>

#include <QtDebug>

namespace Utopia
{

    FileFixerDialog::FileFixerDialog(Parser::Context ctx_, const QString& fileName_, FileFormat* fileFormat_, ExportPolicy exportPolicy)
        : _ctx(ctx_), _fileFormat(fileFormat_), _fileName(fileName_), _exportPolicy(exportPolicy)
    {
        // Set up user interface
        this->_ui = new Ui::FileFixerDialog;
        this->_ui->setupUi(this);

        // Take QIODevice, and write out a temporary file
        QFile file(fileName_);
        file.open(QIODevice::ReadOnly);
        this->_tempFile = new QTemporaryFile();
        this->_tempFile->open();
        while (!file.atEnd())
        {
            this->_tempFile->write(file.read(2048));
        }

        // Load file into editor
        this->_tempFile->seek(0);
        this->_ui->text->setPlainText(this->_tempFile->readAll());

        // Connect signals
        QObject::connect(this->_ui->retryButton, SIGNAL(pressed()), this, SLOT(retry()));
        QObject::connect(this->_ui->cancelButton, SIGNAL(pressed()), this, SLOT(reject()));
        QObject::connect(this->_ui->errors, SIGNAL(clicked(const QModelIndex&)), this, SLOT(goTo(const QModelIndex&)));

        // Assign context model
        this->_ui->errors->setModel(new ParserContextModel(this->_ctx));
        this->_ui->errors->setCurrentIndex(this->_ui->errors->model()->index(0, 0));

        // Goto first line
        this->goTo(this->_ui->errors->model()->index(0, 0));
    }

    FileFixerDialog::~FileFixerDialog()
    {
        delete this->_ui->errors->model();
        delete this->_tempFile;
        delete this->_ui;
    }

    // Accessor
    Parser::Context FileFixerDialog::ctx() const
    {
        return this->_ctx;
    }

    QString FileFixerDialog::exportFileName() const
    {
        return this->_exportFileName;
    }

    // Goto Error/Warning
    void FileFixerDialog::goTo(const QModelIndex& index_)
    {
        int line = 0;
        QString details;
        QString message;
        if (this->_ctx.errorCode() != Parser::None && index_.row() == this->_ctx.warnings().size())
        {
            line = this->_ctx.errorLine();
            details += "<span style=\"font-weight: bold; color: #f00;\">ERROR:</span><br />\n";
            message = this->_ctx.message();
        }
        else
        {
            line = this->_ctx.warnings().at(index_.row()).line;
            details += "<span style=\"font-weight: bold; color: #bb0;\">WARNING:</span><br />\n";
            message = this->_ctx.warnings().at(index_.row()).message;
        }
        details += QString("line: %1<br />\n").arg(line);
        details += QString("%1").arg(message);

        // Change Cursor
        QTextCursor cursor = this->_ui->text->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        this->_ui->text->setTextCursor(cursor);

        // Display Details
        this->_ui->details->setText(details);
    }

    void FileFixerDialog::retry()
    {
        // Clear details
        this->_ui->details->clear();

        // Reflect contents in file
        delete this->_tempFile;
        this->_tempFile = new QTemporaryFile();
        this->_tempFile->open();
        this->_tempFile->write(this->_ui->text->toPlainText().toUtf8());
        this->_tempFile->flush();

        // Try to parse again
        this->_ctx = load(this->_tempFile->fileName(), this->_fileFormat);
        this->_ui->errors->reset();
        this->_ui->errors->scrollToTop();

        // If successful, then bail.
        if (this->_ctx.model() != 0 && this->_ctx.errorCode() == Parser::None)
        {
            QString title = this->_exportPolicy & UseSameFileName ? "Overwrite data file?" : "Save changes?";
            // Save changes?
            if ((this->_exportPolicy & ForceExport) ||
                QMessageBox::question(this, title, "Would you like to save the changes you just made to disk?",
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                QFileInfo originalFile(this->_fileName);
                if (this->_exportPolicy & UseSameFileName)
                {
                    this->_exportFileName = this->_fileName;
                }
                else
                {
                    this->_exportFileName = getExportFileName(this->_fileFormat, this, "Save Changes To...", originalFile.absoluteFilePath());
                }
                if (!this->_exportFileName.isEmpty())
                {
                    QFile file(this->_exportFileName);
                    file.open(QIODevice::WriteOnly);
                    file.write(this->_ui->text->toPlainText().toUtf8());
                }
            }

            this->accept();
        }
        else
        {
            this->_ui->errors->setCurrentIndex(this->_ui->errors->model()->index(0, 0));
            this->goTo(this->_ui->errors->model()->index(0, 0));
        }
    }

} // namespace Utopia
