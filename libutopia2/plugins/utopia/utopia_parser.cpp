/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include "utopia_parser.h"

#include <utopia2/node.h>
#include <utopia2/aminoacid.h>
#include <utopia2/nucleotide.h>

#include <vector>
#include <QTextStream>

#include <raptor.h>

namespace Utopia
{

    //
    // UTOPIAParser
    //

    // Constructor
    UTOPIAParser::UTOPIAParser()
        : Parser()
    {}

    // Valid residues?
    bool UTOPIAParser::valid_residues(const std::string& line_)
    {
        // Valid characters
        static const std::string valid = "abcdefghiklmnpqrstuvwxyzABCDEFGHIKLMNPQRSTUVWXYZ- ";

        return line_.find_first_not_of(valid) == std::string::npos;
    }
    std::string UTOPIAParser::remove_whitespace(const std::string& line_)
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
    void UTOPIAParser::convertResidueSequenceToNodes(const std::string& sequence_str_, Node* sequence_)
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

    struct utopia_parser_state
    {
        Node* authority;
        QMap< int, Node* > nodeIDs;
    };

    static Node* resolveNode(struct utopia_parser_state* state, const void* const data, raptor_identifier_type type)
    {
        switch (type)
        {
        case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        {
            qDebug() << "RESOURCE";
            return Node::getNode((char*) data);
            break;
        }
        case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        {
            int id = 0;
            sscanf((char*) data, "ID%08d", &id);
            if (state->nodeIDs.contains(id))
            {
                return state->nodeIDs[id];
            }
            break;
        }
        default:
            break;
        }

        return 0;
    }

    static void instantiateStatement(void* user_data,
                                     const raptor_statement* statement)
    {
        Node* subject = 0;
        Node* predicate = 0;
        Node* object = 0;
        struct utopia_parser_state* state = (struct utopia_parser_state*) user_data;

        // Special case of an authority
        if (state->authority == 0)
        {
            state->authority = createAuthority();
            return;
        }

        if (statement->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE
            || statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
        {
            subject = resolveNode(state, statement->subject, statement->subject_type);
        }

        if (statement->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE
            || statement->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
        {
            object = resolveNode(state, statement->object, statement->object_type);
        }

        if (statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
        {
            predicate = resolveNode(state, statement->predicate, statement->predicate_type);
            qDebug() << predicate;
            qDebug() << (predicate->attributes.get(UtopiaSystem.uri).toString());
        }

        qDebug() << (char*) statement->subject << (char*) statement->predicate << (char*) statement->object;
    }

    // Parse!
    Node* UTOPIAParser::parse(Parser::Context& ctx, QIODevice& stream_) const
    {
        Parser * ntriples = instantiateExtension< Parser >("Utopia::NTriplesParser");

        // Ensure valid stream
        if (ntriples == 0)
        {
            ctx.setErrorCode(Incapable);
            ctx.setMessage("No N-Triples parser found");
            return 0;
        }

        return ntriples->parse(ctx, stream_);
    }

    QString UTOPIAParser::description() const
    {
        return "UTOPIA";
    }

    QSet< FileFormat* > UTOPIAParser::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* utopia = FileFormat::create("UTOPIA", SequenceFormat);
        *utopia << "utopia";
        formats << utopia;
        return formats;
    }


} // namespace Utopia
