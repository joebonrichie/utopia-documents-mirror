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

#include "pdf_parser.h"

#include <utopia2/node.h>
#include <utopia2/aminoacid.h>
#include <utopia2/nucleotide.h>

#include <spine/Document.h>
#include <papyro/documentfactory.h>

#include <vector>
#include <QTemporaryFile>
#include <QTextStream>
#include <QFile>

namespace Utopia
{

    //
    // PDFParser
    //

    // Constructor
    PDFParser::PDFParser()
        : Parser()
    {}

    // Parse!
    Node* PDFParser::parse(Parser::Context& ctx, QIODevice& stream_) const
    {
        QTemporaryFile tmp;

        // Ensure valid stream
        if (!stream_.isOpen() || !stream_.isReadable())
        {
            ctx.setErrorCode(StreamError);
            ctx.setMessage("Invalid Stream");
        }

        // Ensure not empty stream
        if (stream_.atEnd())
        {
            ctx.setErrorCode(StreamEmpty);
            ctx.setMessage("Empty Stream");
        }

        QFile * file = dynamic_cast< QFile * >(&stream_);

        // Ensure of type file
        if (file == 0)
        {
            // Save to temporary file FIXME
            tmp.open();
            tmp.write(stream_.readAll());
            tmp.close();
            tmp.open();
            file = &tmp;
            tmp.setAutoRemove(false);
        }

        QString pdfFile = file->fileName();

        Node * c_Document = UtopiaDomain.term("Document");

        Node * authority = Utopia::createAuthority();
        Node * model = Utopia::createNode(authority, c_Document);
        authority->relations(Utopia::UtopiaSystem.hasPart).append(model);

        QString fileName = pdfFile;
        model->attributes.set("papyro:pdfFile", fileName);

        Spine::DocumentHandle doc = Papyro::DocumentFactory::load(model);

        // If failed to open
        if (!doc) {
            ctx.setErrorCode(SyntaxError);
            ctx.setMessage("Unknown file format");
        }

        return authority;
    }

    QString PDFParser::description() const
    {
        return "PDF";
    }

    QSet< FileFormat* > PDFParser::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* pdf = FileFormat::create("PDF", DocumentFormat);
        *pdf << "pdf";
        formats << pdf;
        return formats;
    }


} // namespace Utopia
