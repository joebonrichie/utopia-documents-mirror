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

#include "utopia_serializer.h"

#include <utopia2/node.h>
#include <utopia2/aminoacid.h>
#include <utopia2/nucleotide.h>
#include <utopia2/list.h>

#include <raptor.h>
#include <string.h>

#include <QtDebug>
#include <iostream>

namespace Utopia
{

    int qiodevice_iostream_init(void *context)
    {
        return 0;
    }

    void qiodevice_iostream_finish(void *context)
    {}

    int qiodevice_iostream_write_byte(void *context,
                                      const int byte)
    {
        QIODevice* io = (QIODevice*) context;
        return (io->putChar(byte) ? 0 : 1);
    }

    int qiodevice_iostream_write_bytes(void *context,
                                       const void *ptr,
                                       size_t size,
                                       size_t nmemb)
    {
        QIODevice* io = (QIODevice*) context;
        return (io->write((const char*) ptr, size * nmemb) >= 0 ? 0 : 1);
    }

    void qiodevice_iostream_write_end(void *context)
    {}

    QString encodeUnicode(const QString & unicode)
    {
        QString encoded;
        for (int i = 0; i < unicode.length(); ++i)
        {
            QChar ch = unicode[i];
            if (ch == ch.toAscii())
            {
                encoded += ch;
            }
            else
            {
                encoded += "\\u" + QString("%1").arg((ushort) ch.unicode(), (int) 4, (int) 16, QChar('0')).toUpper();
            }
        }
        return encoded;
    }

    static int getNodeID(QMap< Node*, int >& nodeIDs, Node* node)
    {
        static int nodeID = 1;
        if (nodeIDs.contains(node))
        {
            return nodeIDs[node];
        }
        else
        {
            return (nodeIDs[node] = nodeID++);
        }
    }

    static QPair< const void*, raptor_identifier_type > convertNode(QMap< Node*, int >& nodeIDs, Node* node)
    {
        QPair< const void*, raptor_identifier_type > pair;
        if (node->attributes.exists(UtopiaSystem.uri))
        {
            QString encoded = encodeUnicode(node->attributes.get(UtopiaSystem.uri).toString());
            pair.first = raptor_new_uri((const unsigned char*) encoded.toAscii().data());
            pair.second = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        }
        else
        {
            char nodeID[11];
            sprintf(nodeID, "ID%08d", getNodeID(nodeIDs, node));
            pair.first = (const unsigned char*) strdup(nodeID);
            pair.second = RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        }
        return pair;
    }

    static void serialiseNode(raptor_serializer* rdf_serializer, QMap< Node*, int >& nodeIDs, Node* node_, bool auth = false)
    {
        raptor_statement statement = {0};

        if (!auth)
        {
            QPair< const void*, raptor_identifier_type > pair;

            // Serialize node type
            pair = convertNode(nodeIDs, node_);
            statement.subject = pair.first;
            statement.subject_type = pair.second;
            QString encoded = encodeUnicode(rdf.type->attributes.get(UtopiaSystem.uri).toString());
            statement.predicate = raptor_new_uri((const unsigned char*) encoded.toAscii().data());
            statement.predicate_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            pair = convertNode(nodeIDs, node_->type());
            statement.object = pair.first;
            statement.object_type = pair.second;
            raptor_serialize_statement(rdf_serializer, &statement);

            // Serialize attributes
            QList< Node* > attributes = node_->attributes.keys();
            QListIterator< Node* > attributeIterator(attributes);
            while (attributeIterator.hasNext())
            {
                Node* predicateNode = attributeIterator.next();
                QPair< const void*, raptor_identifier_type > pair = convertNode(nodeIDs, predicateNode);
                statement.predicate = pair.first;
                statement.predicate_type = pair.second;

                QString encoded = encodeUnicode(node_->attributes.get(predicateNode).toString());
                statement.object = strdup(encoded.toStdString().c_str());
                statement.object_type = RAPTOR_IDENTIFIER_TYPE_LITERAL;
                const char* uri = 0;
                switch (node_->attributes.get(predicateNode).type())
                {
                case QVariant::Int:
                case QVariant::UInt:
                case QVariant::LongLong:
                case QVariant::ULongLong:
                    uri = "http://www.w3.org/2001/XMLSchema#" "integer";
                    break;
                case QVariant::Double:
                    uri = "http://www.w3.org/2001/XMLSchema#" "double";
                    break;
                case QVariant::Bool:
                    uri = "http://www.w3.org/2001/XMLSchema#" "boolean";
                    break;
                case QVariant::ByteArray:
                    uri = "http://www.w3.org/2001/XMLSchema#" "base64Binary";
                    break;
                case QVariant::Date:
                case QVariant::Time:
                case QVariant::DateTime:
                    uri = "http://www.w3.org/2001/XMLSchema#" "dateTime";
                    break;
                case QVariant::StringList:
                    uri = "http://www.w3.org/2001/XMLSchema#" "string*";
                    break;
                case QVariant::String:
                case QVariant::Url:
                default:
                    uri = "http://www.w3.org/2001/XMLSchema#" "string";
                    break;
                }
                statement.object_literal_datatype = raptor_new_uri((const unsigned char*) uri);
                raptor_serialize_statement(rdf_serializer, &statement);
            }
        }

        if (node_->minions())
        {
            List::iterator iter = node_->minions()->begin();
            List::iterator end = node_->minions()->end();
            for (; iter != end; ++iter)
            {
                serialiseNode(rdf_serializer, nodeIDs, *iter);

                QPair< const void*, raptor_identifier_type > pair = convertNode(nodeIDs, *iter);
                statement.subject = pair.first;
                statement.subject_type = pair.second;

                QList< Property > properties = (*iter)->relations();
                QMutableListIterator< Property > property(properties);
                while (property.hasNext())
                {
                    Property prop = property.next();
                    Node* node = prop.data();
                    QPair< const void*, raptor_identifier_type > pair = convertNode(nodeIDs, node);
                    statement.predicate = pair.first;
                    statement.predicate_type = pair.second;

                    Node::relation::iterator rel_iter = (*iter)->relations(prop).begin();
                    Node::relation::iterator rel_end = (*iter)->relations(prop).end();
                    for (; rel_iter != rel_end; ++rel_iter)
                    {
                        QPair< const void*, raptor_identifier_type > pair = convertNode(nodeIDs, *rel_iter);
                        statement.object = pair.first;
                        statement.object_type = pair.second;

                        raptor_serialize_statement(rdf_serializer, &statement);
                    }
                }
            }
        }
    }

    bool UTOPIASerializer::serialize(Serializer::Context& ctx, QIODevice& stream_, Node* node_) const
    {
        QMap< Node*, int > nodeIDs;

        raptor_iostream_handler2 QIODeviceHandler = {
            2,
            qiodevice_iostream_init,
            qiodevice_iostream_finish,
            qiodevice_iostream_write_byte,
            qiodevice_iostream_write_bytes,
            qiodevice_iostream_write_end,
            0,
            0
        };

        // FIXME base uri?
//        raptor_uri* base_uri = raptor_new_uri((const unsigned char*) "HAHAHA");

//         raptor_serializer* rdf_serializer = raptor_new_serializer("rdfxml");
        raptor_serializer* rdf_serializer = raptor_new_serializer("ntriples");
        raptor_iostream* qiodevice_iostream = raptor_new_iostream_from_handler2(&stream_, &QIODeviceHandler);
        raptor_serialize_set_namespace(rdf_serializer, raptor_new_uri((const unsigned char*) "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system#"), (const unsigned char*) "system");
        raptor_serialize_set_namespace(rdf_serializer, raptor_new_uri((const unsigned char*) "http://utopia.cs.manchester.ac.uk/2007/03/utopia-domain#"), (const unsigned char*) "domain");
        raptor_serialize_start(rdf_serializer, 0 /*base_uri*/, qiodevice_iostream);

        serialiseNode(rdf_serializer, nodeIDs, node_, true);

        raptor_serialize_end(rdf_serializer);

        return true;
    }

    QString UTOPIASerializer::description() const
    {
        return "UTOPIA";
    }

    QSet< FileFormat* > UTOPIASerializer::formats() const
    {
        QSet< FileFormat* > formats;
        FileFormat* utopia = FileFormat::create("UTOPIA", SequenceFormat);
        *utopia << "utopia";
        formats << utopia;
        return formats;
    }

} // namespace Utopia
