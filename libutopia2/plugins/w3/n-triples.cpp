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

#include "n-triples.h"

#include <utopia2/node.h>

#include <QRegExp>
#include <QTextStream>

namespace Utopia
{

    //
    // NTriplesParser
    //

    // Constructor
    NTriplesParser::NTriplesParser()
        : Parser()
    {}

    // Helper methods
    bool NTriplesParser::_can_ignore(QString line_)
    {
        // Check if line is ignorable
        return line_.isEmpty() || line_[0] == '#';
    }
    QChar NTriplesParser::_gobble(QString& line_)
    {
        // Gobble one character / escaped character
        int characters_gobbled = 1;
        QChar gobbled;
        if (line_[0] == '\\' && line_[1] == '\\')
        {
            switch (line_[2].toLatin1())
            {
            case 'u':
                characters_gobbled = 7;
                break;
            case 'U':
                characters_gobbled = 11;
                break;
            default:
                characters_gobbled = 3;
                break;
            }
        }
        if (characters_gobbled > 1)
        {
            switch (line_[2].toLatin1())
            {
            case 'u':
            {
                bool ok;
                gobbled = QChar(line_.mid(3, 4).toUInt(&ok, 16));
                break;
            }
            case 'U':
            {
                bool ok;
                gobbled = QChar(line_.mid(3, 8).toUInt(&ok, 16));
                break;
            }
            case 'n':
                gobbled =  '\n';
            case 'r':
                gobbled =  '\r';
            case 't':
                gobbled =  '\t';
            case '"':
                gobbled =  '"';
            case '\\':
                gobbled =  '\\';
            default:
                gobbled =  line_[2];
            }
        }
        else
        {
            gobbled = line_[0];
        }
        line_ = line_.mid(characters_gobbled);
        return gobbled;
    }
    QString NTriplesParser::_strip_ns(QString& uriref_)
    {
        int lastDelimiter = uriref_.lastIndexOf("/#");
        if (lastDelimiter == -1)
        {
            return "";
        }
        else
        {
            QString ns = uriref_.mid(0, lastDelimiter + ((uriref_.at(lastDelimiter) == '/') ? 1 : 0));
            uriref_ = uriref_.mid(lastDelimiter + 1);
            return ns;
        }
    }

    // Parse!
    Node* NTriplesParser::parse(Parser::Context& ctx, QIODevice& stream_) const
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

        // Parser's current state (per line)
        enum {
            Subject = 0,
            Predicate,
            Object,
            Finished
        };
        int triple = Subject;
        enum part {
            Unknown,
            UriRef,
            NodeID,
            Literal,
            Type,
            Lang
        } part = Unknown;
        int line_no = 0;

        // Create authority thing for this ontology
        Node* thing = createAuthority();
        bool first = true;

        // Terms
        QMap< QString, Node* > terms;
        QMap< QString, Node* > namedNodes;

        // Read file, line by line
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            ++line_no;
            // Ignore empty lines
            if (_can_ignore(line))
            {
                continue;
            }

            // Variables for possible things
            struct {
                enum part type;
                QString str;
                QString type_str;
                QString lang_str;
            } tokens[3];

            // Go through each line atom by atom
            part = Unknown;
            triple = Subject;
            QChar atom;
            while (!line.isEmpty() && !line.isEmpty()) // FIXME Why twice!?
            {
                atom = _gobble(line);
                switch (part)
                {
                case Unknown:
                    switch (atom.toLatin1())
                    {
                    case '<':
                        tokens[triple].type = part = UriRef;
                        break;
                    case '_':
                        tokens[triple].type = NodeID;
                        tokens[triple].str = line.mid(1, line.indexOf(QRegExp("[^a-zA-Z0-9]"), 1) - 1);
                        line = line.mid(tokens[triple].str.length() + 1);
                        ++triple;
                        break;
                    case '"':
                        tokens[triple].type = part = Literal;
                        break;
                    case ' ':
                    case '\t':
                        break;
                    default:
                        // Unexpected character found in stream
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        // FIXME include character position, and maybe the line itself
                        ctx.setMessage(QString("Unexpected character '") + atom + "' found in stream.");
                        delete thing;
                        return 0;
                        break;
                    }
                    break;
                case Type:
                case UriRef:
                    if (atom == '>')
                    {
                        part = Unknown;
                        ++triple;
                    }
                    else
                    {
                        if (part == Type)
                        {
                            tokens[triple].type_str += atom;
                        }
                        else
                        {
                            tokens[triple].str += atom;
                        }
                    }
                    break;
                case Literal:
                    if (atom == '"')
                    {
                        if (!line.isEmpty() && line[0] == '^')
                        {
                            if (line.length() > 2 && line[1] == '^' && line[2] == '<')
                            {
                                part = Type;
                            }
                            else
                            {
                                // Unexpected character found in stream
                                ctx.setErrorCode(SyntaxError);
                                ctx.setErrorLine(line_no);
                                ctx.setMessage(QString("Unexpected character '") + atom + "' found in stream.");
                                delete thing;
                                return 0;
                            }
                        }
                        else if (!line.isEmpty() && line[0] == '@')
                        {
                            tokens[triple].lang_str = line.mid(1, line.indexOf(QRegExp("[^a-z0-9-]"), 1));
                            line = line.mid(tokens[triple].lang_str.length());
                            part = Unknown;
                            ++triple;
                        }
                        else
                        {
                            part = Unknown;
                            ++triple;
                        }
                    }
                    else
                    {
                        tokens[triple].str += atom;
                    }
                    break;
                case NodeID:
                case Lang:
                default:
                    // Lang and NodeID will never occur
                    break;
                }

                // Parsing of this line is finished
                if (triple == Finished)
                {
                    if (line != "")
                    {
                        // Any more input on this line is ignored
                        QString warning = "Some trailing characters were ignored: ";
                        ctx.addWarning(warning + line, line_no);
                    }

                    // Begin by resolving the predicate
                    QString predicate_uri = tokens[Predicate].str;
                    Node* predicate = 0;
                    if (tokens[Predicate].type == UriRef)
                    {
                        // Tokenise URI
                        QString predicate_id = predicate_uri;
                        QString predicate_ns = _strip_ns(predicate_id);

                        // Get ontology for this predicate
                        Ontology predicateOntology = Ontology::fromURI(predicate_ns, true);

                        // Find term in cache?
                        QMap< QString, Node* >::iterator found_term = terms.find(predicate_uri);
                        if (found_term != terms.end())
                        {
                            // Found in the cache!
                            predicate = found_term.value();
                        }
                        else
                        {
                            // Not found in cache. How about in the model?
                            predicate = predicateOntology.term(predicate_id);
                            if (predicate != 0)
                            {
                                // Found in the model!
                                terms[predicate_uri] = predicate;
                            }
                            else
                            {
                                // Not found in the model either. Create one.
                                // N.B. This is definitely a Property!
                                terms[predicate_uri] = predicate = createProperty(predicateOntology);
                                predicate->attributes.set(UtopiaSystem.uri, predicate_uri);
                            }
                        }
                    }
                    else
                    {
                        // nodeID or literal found where not allowed
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage(QString("URI reference expected as statement predicate, but found ") + (tokens[Predicate].type == NodeID ? "named node" : "literal") + ".");
                        delete thing;
                        return 0;
                    }

                    // Now resolve the subject
                    QString subject_uri = tokens[Subject].str;
                    Node* subject = 0;
                    if (tokens[Subject].type == UriRef)
                    {
                        // Tokenise URI
                        QString subject_id = subject_uri;
                        QString subject_ns = _strip_ns(subject_id);

                        // Get ontology for this subject
                        Ontology subjectOntology = Ontology::fromURI(subject_ns, true);

                        // Find term in cache?
                        QMap< QString, Node* >::iterator found_term = terms.find(subject_uri);
                        if (found_term != terms.end())
                        {
                            // Found in the cache!
                            subject = found_term.value();
                        }
                        else
                        {
                            // Not found in cache. How about in the model?
                            subject = subjectOntology.term(subject_id);
                            if (subject != 0)
                            {
                                // Found in the model!
                                terms[subject_uri] = subject;
                            }
                            else
                            {
                                // Not found in the model either. Create one.
                                // This may be a Property or a class.
                                terms[subject_uri] = subject = createNode(subjectOntology);
                                subject->attributes.set(UtopiaSystem.uri, subject_uri);
                            }
                        }
                    }
                    else if (tokens[Subject].type == NodeID)
                    {
                        // Find Node in cache?
                        QMap< QString, Node* >::iterator found_namedNode = namedNodes.find(tokens[Subject].str);
                        if (found_namedNode != namedNodes.end())
                        {
                            // Found in the cache!
                            subject = found_namedNode.value();
                        }
                        else
                        {
                            namedNodes[tokens[Subject].str] = subject = thing->create();
                        }
                    }
                    else
                    {
                        // literal found where not allowed
                        ctx.setErrorCode(SyntaxError);
                        ctx.setErrorLine(line_no);
                        ctx.setMessage("Expected URI reference or named node as statement subject, but found literal.");
                        delete thing;
                        return 0;
                    }
                    if (first)
                    {
                        thing->relations(UtopiaSystem.hasPart).append(subject);
                        first = false;
                    }

                    // Now resolve the object
                    QString object_uri = tokens[Object].str;
                    Node* object = 0;
                    if (tokens[Object].type == UriRef)
                    {
                        // Tokenise URI
                        QString object_id = object_uri;
                        QString object_ns = _strip_ns(object_id);

                        // Get ontology for this object
                        Ontology objectOntology = Ontology::fromURI(object_ns, true);

                        // Find term in cache?
                        QMap< QString, Node* >::iterator found_term = terms.find(object_uri);
                        if (found_term != terms.end())
                        {
                            // Found in the cache!
                            object = found_term.value();
                        }
                        else
                        {
                            // Not found in cache. How about in the model?
                            object = objectOntology.term(object_id);
                            if (object != 0)
                            {
                                // Found in the model!
                                terms[object_uri] = object;
                            }
                            else
                            {
                                // Not found in the model either. Create one.
                                // This may be a Property or a class.
                                terms[object_uri] = object = createNode(objectOntology);
                                object->attributes.set(UtopiaSystem.uri, object_uri);
                            }
                        }

                        // Deal with statement
                        if (predicate == rdf.type)
                        {
                            subject->setType(object);
                        }
                        else
                        {
                            subject->relations(predicate).append(object);
                        }
                    }
                    else if (tokens[Object].type == NodeID)
                    {
                        // Find Node in cache?
                        QMap< QString, Node* >::iterator found_namedNode = namedNodes.find(tokens[Object].str);
                        if (found_namedNode != namedNodes.end())
                        {
                            // Found in the cache!
                            object = found_namedNode.value();
                        }
                        else
                        {
                            namedNodes[tokens[Object].str] = object = thing->create();
                        }

                        // Deal with statement
                        if (predicate == rdf.type)
                        {
                            subject->setType(object);
                        }
                        else
                        {
                            subject->relations(predicate).append(object);
                        }
                    }
                    else if (tokens[Object].type == Literal)
                    {
                        // Set attribute
                        subject->attributes.set(predicate_uri, object_uri);
                    }

                    break;
                }
            }
        }

        // If ontology is empty, then delete authority and return 0
        if (thing->minions()->empty())
        {
            delete thing;
            thing = 0;
        }

        return thing;
    }

    QString NTriplesParser::description() const
    {
        return "N-Triples";
    }

    QSet< FileFormat* > NTriplesParser::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* ntriples = FileFormat::create("N-Triples", OntologyFormat);
        *ntriples << "nt" << "ntriples";
        formats << ntriples;
        return formats;
    }

} // namespace Utopia
