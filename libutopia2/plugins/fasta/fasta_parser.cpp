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

#include "fasta_parser.h"

#include <utopia2/node.h>
#include <utopia2/aminoacid.h>
#include <utopia2/nucleotide.h>

#include <vector>
#include <QTextStream>

namespace Utopia
{

    //
    // FASTAParser
    //

    // Constructor
    FASTAParser::FASTAParser()
        : Parser()
    {}

    // Valid residues?
    bool FASTAParser::valid_residues(const std::string& line_)
    {
        // Valid characters
        static const std::string valid = "abcdefghiklmnpqrstuvwxyzABCDEFGHIKLMNPQRSTUVWXYZ- ";

        return line_.find_first_not_of(valid) == std::string::npos;
    }
    std::string FASTAParser::remove_whitespace(const std::string& line_)
    {
        // Valid characters
        static const std::string valid = "abcdefghiklmnpqrstuvwxyzABCDEFGHIKLMNPQRSTUVWXYZ-";
        std::string output;

        for (size_t i = 0; i < line_.length(); ++i)
        {
            if (valid.find(line_.at(i)) != std::string::npos)
            {
                output += line_.at(i);
            }
        }

        return output;
    }
    void FASTAParser::convertResidueSequenceToNodes(const std::string& sequence_str_, Node* sequence_)
    {
        Node* p_code = UtopiaDomain.term("code");
        Node* p_size = UtopiaDomain.term("size");
        Node* c_Gap = UtopiaDomain.term("Gap");

        // For each letter, presume Nucleotide
        bool Nucleotide = true;
        std::vector< Node* > residues;
        std::vector< int > gaps;
        int gap = 0;
        for (size_t i = 0; i < sequence_str_.size(); ++i)
        {
            // Make code string
            std::string code = sequence_str_.substr(i, 1);

            if (sequence_str_.at(i) == '-')
            {
                ++gap;
                continue;
            }

            Node* residue = 0;

            if (Nucleotide)
            {
                residue = Nucleotide::get(QString::fromStdString(code));

                // If residue is not a Nucleotide, then re-evaluate...
                if (residue == 0)
                {
                    // Re-evaluate List as amino acids
                    for (size_t j = 0; j < residues.size(); ++j)
                    {
                        residue = AminoAcid::get(residues[j]->attributes.get(p_code).toString());
                        residues[j] = residue;
                    }

                    Nucleotide = false;
                }
                else
                {
                    residues.push_back(residue);
                }
            }

            if (!Nucleotide)
            {
                residue = AminoAcid::get(QString::fromStdString(code));
                residues.push_back(residue);
            }

            // Attach gaps?
            gaps.push_back(gap);
            if (gap > 0)
            {
                gap = 0;
            }
        }

        for (size_t i = 0; i < residues.size(); ++i)
        {
            Node* residue = sequence_->create(residues.at(i));
            sequence_->relations(UtopiaSystem.hasPart).append(residue);
            if (gaps.at(i) > 0)
            {
                Node* gap_annotation = sequence_->create(c_Gap);
                gap_annotation->relations(UtopiaSystem.annotates).append(residue);
                gap_annotation->attributes.set(p_size, QVariant::fromValue(gaps.at(i)));
            }
        }
    }

    // Parse!
    Node* FASTAParser::parse(Parser::Context& ctx, QIODevice& stream_) const
    {
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

        QTextStream stream(&stream_);

        // State machine state
        enum {
            Header,
            Comment,
            Sequence,
            Finished
        } state = Header;
        size_t line_no = 0;

        // List of nodes
        std::vector< Node* > authorities;
        std::vector< Node* > sequences;

        // Convenience...
        Node* p_description = UtopiaDomain.term("description");
        Node* p_title = UtopiaDomain.term("title");
        Node* c_Alignment = UtopiaDomain.term("Alignment");
        Node* c_Sequence = UtopiaDomain.term("Sequence");
        Node* sequence = 0;
        std::string sequence_str;
        Node* authority = 0;

        // Read file, line by line
        while (state != Finished && !stream.atEnd()) {
            QString qline = stream.readLine().trimmed();
            ++line_no;
            // Ignore empty lines
            if (qline.isEmpty())
            {
                continue;
            }
            std::string line = qline.toStdString();

            // State machine
            switch (state)
            {
            case Comment:
                if (line.at(0) == ';')
                {
                    break;
                }
                else
                {
                    state = Sequence;
                }
            case Sequence:
                // Parse sequence
                if (line.at(0) == '>')
                {
                    if (sequence_str == "")
                    {
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Sequence data missing.");
                        state = Finished;
                        break;
                    }
                    else
                    {
                        state = Header;
                        convertResidueSequenceToNodes(sequence_str, sequence);
                    }
                }
                else if (this->valid_residues(line))
                {
                    sequence_str += this->remove_whitespace(line);
                    break;
                }
                else
                {
                    ctx.setErrorCode(SyntaxError);
                    ctx.setErrorLine(line_no);
                    ctx.setMessage("Unexpected characters found parsing sequence data.");
                    state = Finished;
                    break;
                }
            case Header:
                if (line.at(0) == '>')
                {
                    // Deal with no title
                    if (line.size() == 1 || (line = QString::fromStdString(line.substr(1)).trimmed().toStdString()) == "")
                    {
                        ctx.setErrorCode(UnexpectedEol);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Sequence title not found after '>'.");
                        state = Finished;
                        break;
                    }
                    // Parse header
                    else
                    {
                        // Reset sequence string
                        sequence_str = "";

                        // get title + description
                        size_t ws = line.find_first_of(" \t");
                        std::string title;
                        std::string description;
                        if (ws == std::string::npos)
                        {
                            title = line;
                        }
                        else
                        {
                            title = line.substr(0, ws);
                            description = QString::fromStdString(line.substr(ws)).trimmed().toStdString();
                        }

                        // Create nodes
                        authority = createAuthority();
                        sequence = authority->create(c_Sequence);
                        authority->relations(UtopiaSystem.hasPart).append(sequence);

                        // Describe
                        authority->attributes.set(p_title, title.c_str());
                        authority->attributes.set(p_description, description.c_str());
                        sequence->attributes.set(p_title, title.c_str());
                        sequence->attributes.set(p_description, description.c_str());

                        // Collect
                        authorities.push_back(authority);
                        sequences.push_back(sequence);
                    }

                    state = Comment;
                }
                else
                {
                    ctx.setErrorCode(SyntaxError);
                    ctx.setErrorLine(line_no);
                    ctx.setMessage("Expected header line but found different.");
                    state = Finished;
                }
                break;
            case Finished:
                // This will never happen
                break;
            }
        }

        if (ctx.errorCode() == None)
        {
            if (authority == 0)
            {
                ctx.setErrorCode(SyntaxError);
                ctx.setMessage("No sequences found.");
            }
            else if (sequence_str == "")
            {
                ctx.setErrorCode(SyntaxError);
                ctx.setErrorLine(line_no);
                ctx.setMessage("Sequence data missing.");
            }
            else
            {
                convertResidueSequenceToNodes(sequence_str, sequence);

                // Deal with single / multiple
                if (authorities.size() > 1) // Muliple
                {
                    // Create uber authority + alignment
                    authority = createAuthority();
                    Node* alignment = authority->create(c_Alignment);

                    // Collect authorities for each sequence
                    for (size_t i = 0; i < authorities.size(); ++i)
                    {
                        authority->relations(UtopiaSystem.hasPart).append(authorities.at(i));
                        authorities.at(i)->setAuthority(authority);
                        alignment->relations(UtopiaSystem.annotates).append(sequences.at(i));
                    }
                }
            }
        }
        // Else clean up
        else if (authority)
        {
            delete authority;
            authority = 0;
        }

        return authority;
    }

    QString FASTAParser::description() const
    {
        return "FastA";
    }

    QSet< FileFormat* > FASTAParser::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* fasta = FileFormat::create("FastA", SequenceFormat);
        *fasta << "fas" << "fasta";
        formats << fasta;
        return formats;
    }


} // namespace Utopia
