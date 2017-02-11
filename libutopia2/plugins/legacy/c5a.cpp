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

#include "c5a.h"

#include <utopia2/node.h>
#include <utopia2/parser.h>
#include <utopia2/aminoacid.h>
#include <utopia2/nucleotide.h>

#include <QTextStream>
#include <string>
#include <sstream>
#include <vector>

namespace Utopia
{

    struct sequence
    {
        std::string title;
        std::string description;
        std::string sequence;
    };
    struct motif
    {
        int left;
        int right;
    };
    struct fingerprint
    {
        std::string name;
        int top;
        int bottom;
        int motif_count;
        std::vector< struct motif > motifs;
    };

    //
    // C5AParser
    //

    // Constructor
    C5AParser::C5AParser()
        : Parser()
    {}

    // Valid residues?
    bool C5AParser::valid_residues(const std::string& line_)
    {
        // Valid characters
        static const std::string valid = "ACDEFGHIKLMNPQRSTUVWXY- ";

        return line_.find_first_not_of(valid) == std::string::npos;
    }
    std::string C5AParser::remove_whitespace(const std::string& line_)
    {
        // Valid characters
        static const std::string valid = "ACDEFGHIKLMNPQRSTUVWXY-";
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
    void C5AParser::convertResidueSequenceToNodes(const std::string& sequence_str_, Node* sequence_)
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
                gap_annotation->attributes.set(p_size, gaps.at(i));
            }
        }
    }

    // Parse!
    Node* C5AParser::parse(Parser::Context& ctx, QIODevice& stream_) const
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
            Section,
            Sequences,
            Breakpoints,
            Fingerprints,
            Finished
        } state = Header;
        size_t line_no = 0;

        // Breakpoints
        std::vector< size_t > breakpoints;

        // Sequences
        enum {
            seq_Chevron,
            seq_Identifier,
            seq_Description,
            seq_Sequence
        } sequence_state = seq_Chevron;
        int sequence_count = -1;
        struct sequence sequence;

        // Fingerprints
        enum {
            fp_Chevron,
            fp_Name,
            fp_Height,
            fp_MotifCount,
            fp_Motifs
        } fingerprint_state = fp_Chevron;
        int fingerprint_count = -1;
        struct fingerprint fingerprint;
        struct motif motif;

        // Convenience...
        Node* p_title = UtopiaDomain.term("title");
        Node* p_description = UtopiaDomain.term("description");
        Node* c_Molecule = UtopiaDomain.term("Molecule");
        Node* c_Sequence = UtopiaDomain.term("Sequence");
        Node* c_Alignment = UtopiaDomain.term("Alignment");
        std::string sequence_str;
        Node* authority = createAuthority();
        Node* alignment = authority->create(c_Alignment);

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
            case Header:
                if (line == "#c5a")
                {
                    state = Section;
                }
                else
                {
                    // C5A header not found
                    ctx.setErrorCode(SyntaxError);
                    ctx.setErrorLine(line_no);
                    ctx.setMessage("File does not start with #c5a header.");
                    state = Finished;
                }
                break;
            case Section:
                if (line == "#end")
                {
                    state = Finished;
                }
                else if (line == "#sequences")
                {
                    state = Sequences;
                }
                else if (line == "#breakpoints")
                {
                    state = Breakpoints;
                }
                else if (line == "#motifs")
                {
                    state = Fingerprints;
                }
                else
                {
                    // Section header not found
                    ctx.setErrorCode(SyntaxError);
                    ctx.setErrorLine(line_no);
                    ctx.setMessage("Syntax error: unexpected characters when expecting section header.");
                    state = Finished;
                }
                break;
            case Sequences:
                if (sequence_count == -1)
                {
                    std::stringstream countstream;
                    countstream << line;
                    countstream >> sequence_count;

                    if (sequence_count == -1)
                    {
                        // Couldn't parse int
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Syntax error: cannot parse sequence count.");
                        state = Finished;
                    }
                }
                else
                {
                    switch (sequence_state)
                    {
                    case seq_Chevron:
                        if (line == ">")
                        {
                            sequence.sequence = "";
                            sequence_state = seq_Identifier;
                        }
                        else
                        {
                            ctx.setErrorCode(SyntaxError);
                            ctx.setErrorLine(line_no);
                            ctx.setMessage("Syntax error: unexpected characters when expecting '>' to delimit the start of a sequence.");
                            state = Finished;
                        }
                        break;
                    case seq_Identifier:
                    {
                        sequence.title = line;
                        sequence_state = seq_Description;
                        break;
                    }
                    case seq_Description:
                    {
                        sequence.description = line;
                        sequence_state = seq_Sequence;
                        break;
                    }
                    case seq_Sequence:
                        if (line == "*")
                        {
                            sequence_state = seq_Chevron;
                            --sequence_count;
                            if (sequence_count == 0)
                            {
                                state = Section;
                            }

                            // Deal with sequence
                            Node* seq_authority = createAuthority(authority);
                            authority->relations(UtopiaSystem.hasPart).append(seq_authority);
                            Node* molecule = seq_authority->create(c_Molecule);
                            seq_authority->relations(UtopiaSystem.hasPart).append(molecule);
                            Node* sequence_node = molecule->create(c_Sequence);
                            molecule->relations(UtopiaSystem.hasPart).append(sequence_node);
                            alignment->relations(UtopiaSystem.annotates).append(sequence_node);

                            // Describe
                            seq_authority->attributes.set(p_title, QString::fromStdString(sequence.title));
                            seq_authority->attributes.set(p_description, QString::fromStdString(sequence.description));
                            sequence_node->attributes.set(p_title, QString::fromStdString(sequence.title));
                            sequence_node->attributes.set(p_description, QString::fromStdString(sequence.description));

                            // Parse residues
                            convertResidueSequenceToNodes(sequence.sequence, sequence_node);
                        }
                        else if (valid_residues(line))
                        {
                            sequence.sequence += remove_whitespace(line);
                        }
                        else
                        {
                            ctx.setErrorCode(SyntaxError);
                            ctx.setErrorLine(line_no);
                            ctx.setMessage("Syntax error: unexpected characters in sequence line.");
                            state = Finished;
                        }
                        break;
                    }
                }
                break;
            case Breakpoints:
                if (line == "*")
                {
                    state = Section;
                }
                else
                {
                    int index = -1;
                    std::stringstream indexstream;
                    indexstream << line;
                    indexstream >> index;
                    if (index >= 0)
                    {
                        breakpoints.push_back((size_t) index);
                    }
                    else
                    {
                        // Couldn't parse index
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Syntax error: cannot parse breakpoint position.");
                        state = Finished;
                    }
                }
                break;
            case Fingerprints:
                if (fingerprint_count == -1)
                {
                    std::stringstream countstream;
                    countstream << line;
                    countstream >> fingerprint_count;

                    if (fingerprint_count == -1)
                    {
                        // Couldn't parse int
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Syntax error: cannot parse sequence count.");
                        state = Finished;
                    }
                }
                else
                {
                    switch (fingerprint_state)
                    {
                    case fp_Chevron:
                        if (line == ">")
                        {
                            fingerprint.motifs.clear();
                            fingerprint_state = fp_Name;
                        }
                        else
                        {
                            ctx.setErrorCode(SyntaxError);
                            ctx.setErrorLine(line_no);
                            ctx.setMessage("Syntax error: unexpected characters when expecting '>' to delimit the start of a fingerprint.");
                            state = Finished;
                        }
                        break;
                    case fp_Name:
                        fingerprint.name = line;
                        fingerprint_state = fp_Height;
                        break;
                    case fp_Height:
                    {
                        std::stringstream parsestream;
                        parsestream << line;
                        fingerprint.top = fingerprint.bottom = -1;
                        parsestream >> fingerprint.top >> fingerprint.bottom;

                        if (fingerprint.top == -1 || fingerprint.bottom == -1)
                        {
                            // Couldn't parse int
                            ctx.setErrorCode(SyntaxError);
                            ctx.setErrorLine(line_no);
                            ctx.setMessage("Syntax error: cannot parse fingerprint span.");
                            state = Finished;
                        }

                        fingerprint_state = fp_MotifCount;
                        break;
                    }
                    case fp_MotifCount:
                    {
                        std::stringstream parsestream;
                        parsestream << line;
                        fingerprint.motif_count = -1;
                        parsestream >> fingerprint.motif_count;

                        if (fingerprint.motif_count == -1)
                        {
                            // Couldn't parse int
                            ctx.setErrorCode(SyntaxError);
                            ctx.setErrorLine(line_no);
                            ctx.setMessage("Syntax error: cannot parse motif count.");
                            state = Finished;
                        }

                        fingerprint_state = fp_Motifs;
                        break;
                    }
                    case fp_Motifs:
                        if (line == "*")
                        {
                            fingerprint_state = fp_Chevron;
                            --fingerprint_count;
                            if (fingerprint_count == 0)
                            {
                                state = Section;
                            }

                            // FIXME Deal with fingerprint
                        }
                        else
                        {
                            std::stringstream parsestream;
                            parsestream << line;
                            motif.left = motif.right - 1;
                            parsestream >> motif.left >> motif.right;

                            if (motif.left == -1 || motif.right == -1)
                            {
                                // Couldn't parse int
                                ctx.setErrorCode(SyntaxError);
                                ctx.setErrorLine(line_no);
                                ctx.setMessage("Syntax error: cannot parse motif extent.");
                                state = Finished;
                            }

                            fingerprint.motifs.push_back(motif);
                        }
                        break;
                    }
                }
                break;
            case Finished:
                // This will never happen
                break;
            }
        }

        return 0;
    }

    QString C5AParser::description() const
    {
        return "C5A";
    }

    QSet< FileFormat* > C5AParser::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* c5a = FileFormat::create("CINEMA5 Alignment", SequenceFormat);
        *c5a << "c5a";
        formats << c5a;
        return formats;
    }

} // namespace Utopia
