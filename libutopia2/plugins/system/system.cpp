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

#include <utopia2/extension.h>
#include <utopia2/extensionlibrary.h>
#include <utopia2/initializer.h>
#include <utopia2/node.h>
#include <utopia2/ontology.h>
#include <utopia2/parser.h>
#include <utopia2/serializer.h>
#include <boost/foreach.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QSettings>
#include <QString>
#include <QMap>
#include <QtDebug>

namespace Utopia
{

    QString joinPath(const QString& s1_, const QString& s2_, const QString& s3_ = "", const QString& s4_ = "", const QString& s5_ = "")
    {
#ifdef _WIN32
        QString sep = "\\";
#else
        QString sep = "/";
#endif
        QString path = s1_ + sep + s2_;
        if (!s3_.isEmpty()) {
            path += sep + s3_;
            if (!s4_.isEmpty()) {
                path += sep + s4_;
                if (!s5_.isEmpty()) {
                    path += sep + s5_;
                }
            }
        }

        return path;
    }

    //
    // SystemInitializer
    //

    class SystemInitializer : public Initializer
    {
    public:
        // Constructor
        SystemInitializer() : Initializer() {};

        void statement(Node* subject_, const Property& predicate_, Node* object_)
        {
            subject_->relations(predicate_).append(object_);
        }

        // Initialize!
        void init()
        {
            this->setMessage("Bootstrapping Utopia Core...");

            // Get N-Triples Parser for system ontologies
            Parser * ntriples = Utopia::instantiateExtension< Parser >("Utopia::NTriplesParser");

            // Open files
            QFile rdf_file(joinPath(Utopia::resource_path(), "ontologies", "rdf.nt"));
            QFile rdfs_file(joinPath(Utopia::resource_path(), "ontologies", "rdfs.nt"));
            QFile owl_file(joinPath(Utopia::resource_path(), "ontologies", "owl.nt"));
            QFile mygrig_file(joinPath(Utopia::resource_path(), "ontologies", "mygrid.nt"));
            QFile domain_file(joinPath(Utopia::resource_path(), "ontologies", "utopia_domain.nt"));
            QFile system_file(joinPath(Utopia::resource_path(), "ontologies", "utopia_system.nt"));

            // If files could not be opened, bail!
            if (!rdf_file.open(QIODevice::ReadOnly))
            {
                setErrorCode(Fatal);
                setMessage(QString("Could not open RDF system ontology: ") + rdf_file.fileName());
                return;
            }
            if (!rdfs_file.open(QIODevice::ReadOnly))
            {
                setErrorCode(Fatal);
                setMessage(QString("Could not open RDFS system ontology: ") + rdfs_file.fileName());
                return;
            }
//                if (!owl_file.open(QIODevice::ReadOnly))
//                {
//                    setErrorCode(Fatal);
//                    setMessage(QString("Could not open OWL system ontology: ") + owl_file.fileName());
//                    return;
//                }
//                if (!mygrid_file.open(QIODevice::ReadOnly))
//                {
//                    setErrorCode(Fatal);
//                    setMessage(QString("Could not open myGrid system ontology: ") + mygrid_file.fileName());
//                    return;
//                }
            if (!system_file.open(QIODevice::ReadOnly))
            {
                setErrorCode(Fatal);
                setMessage(QString("Could not open Utopia system ontology: ") + system_file.fileName());
                return;
            }
            if (!domain_file.open(QIODevice::ReadOnly))
            {
                setErrorCode(Fatal);
                setMessage(QString("Could not open Utopia domain ontology: ") + domain_file.fileName());
                return;
            }


            // Parse ontologies
            this->setMessage("Importing RDF ontology");
            ntriples->parse(rdf_file);
            rdf.Statement = rdf.term("Statement");
            rdf.subject = rdf.term("subject");
            rdf.predicate = rdf.term("predicate");
            rdf.object = rdf.term("object");


            this->setMessage("Importing RDFS ontology");
            ntriples->parse(rdfs_file);
            rdfs.Resource = rdfs.term("Resource");
            rdfs.seeAlso = rdfs.term("seeAlso");
            rdfs.isDefinedBy = rdfs.term("isDefinedBy");
            rdfs.comment = rdfs.term("comment");
            rdfs.label = rdfs.term("label");
            rdfs.Literal = rdfs.term("Literal");
            rdfs.ConstraintResource = rdfs.term("ConstraintResource");
            rdfs.ConstraintProperty = rdfs.term("ConstraintProperty");
            rdfs.range = rdfs.term("range");
            rdfs.domain = rdfs.term("domain");
            rdfs.ContainerMembershipProperty = rdfs.term("ContainerMembershipProperty");


//                this->setMessage("Importing OWL ontology");
//                ntriples->parse(owl_file);
//                myGrid = Node::getNode("http://www.w3.org/2002/07/owl");


//                this->setMessage("Importing myGrid ontology");
//                ntriples->parse(mygrid_file);
//                myGrid = Node::getNode("http://www.mygrid.org.uk/ontology");


            this->setMessage("Importing Utopia system ontology...");
            ntriples->parse(system_file);
            UtopiaSystem.has = UtopiaSystem.term("has");
            UtopiaSystem.hasInput = UtopiaSystem.term("hasInput");
            UtopiaSystem.hasAction = UtopiaSystem.term("hasAction");
            UtopiaSystem.hasOutput = UtopiaSystem.term("hasOutput");
            UtopiaSystem.hasPart = UtopiaSystem.term("hasPart");
            UtopiaSystem.annotates = UtopiaSystem.term("annotates");


            this->setMessage("Importing Utopia domain ontology...");
            ntriples->parse(domain_file);
            UtopiaDomain = Node::getNode("http://utopia.cs.manchester.ac.uk/2007/03/utopia-domain");

            rdf_file.close();
            rdfs_file.close();
//                owl_file.close();
//                mygrig_file.close();
            domain_file.close();
            system_file.close();

            delete ntriples;

#ifdef Q_OS_WIN
            // On WINDOWS, register the utopia(s) protocols
            QString cmdLine(QCoreApplication::applicationFilePath());
            cmdLine = "\"" + cmdLine.replace("/", "\\") + "\" \"%1\"";

            QSettings utopiaKey("HKEY_CURRENT_USER\\Software\\Classes\\utopia", QSettings::NativeFormat);
            utopiaKey.setValue(".", "URL:utopia Protocol");
            utopiaKey.setValue("URL Protocol", "");
            QSettings utopiaOpenKey("HKEY_CURRENT_USER\\Software\\Classes\\utopia\\shell\\open\\command", QSettings::NativeFormat);
            utopiaOpenKey.setValue(".", cmdLine);

            QSettings utopiasKey("HKEY_CURRENT_USER\\Software\\Classes\\utopias", QSettings::NativeFormat);
            utopiasKey.setValue(".", "URL:utopias Protocol");
            utopiasKey.setValue("URL Protocol", "");
            QSettings utopiasOpenKey("HKEY_CURRENT_USER\\Software\\Classes\\utopias\\shell\\open\\command", QSettings::NativeFormat);
            utopiasOpenKey.setValue(".", cmdLine);

            QSettings utopiafileKey("HKEY_CURRENT_USER\\Software\\Classes\\utopiafile", QSettings::NativeFormat);
            utopiafileKey.setValue(".", "URL:utopiafile Protocol");
            utopiafileKey.setValue("URL Protocol", "");
            QSettings utopiafileOpenKey("HKEY_CURRENT_USER\\Software\\Classes\\utopiafile\\shell\\open\\command", QSettings::NativeFormat);
            utopiafileOpenKey.setValue(".", cmdLine);
#endif

            return;
        }

        void final()
        {
        }

        QString description()
        {
            return "System initializer";
        }

    }; // class SystemInitializer

    //
    // ParserInitializer
    //

    class ParserInitializer : public Initializer
    {
    public:
        // Constructor
        ParserInitializer() : Initializer() {};

        // Initialize!
        void init()
            {
                this->setMessage("Registering parsers...");

                BOOST_FOREACH(Parser * parser, Utopia::instantiateAllExtensions< Parser >())
                {
                    this->setMessage(QString("  Registering %1 parser").arg(parser->description()));
                    QSet< FileFormat * > formats = parser->formats();
                    QSetIterator< FileFormat * > i(formats);
                    while (i.hasNext())
                    {
                        FileFormat * format = i.next();
                        if (Parser::get(format))
                        {
                            // FIXME Format already registered
                        }
                        else
                        {
                            Parser::registerFormat(format, parser);
                        }
                    }
                }
            }

        void final()
            {}

        QString description()
            {
                return "Parser initializer";
            }

    }; // class ParserInitializer

    //
    // SerializerInitializer
    //

    class SerializerInitializer : public Initializer
    {
    public:
        // Constructor
        SerializerInitializer() : Initializer() {};

        // Initialize!
        void init()
        {
            this->setMessage("Registering serializers...");

            BOOST_FOREACH(Serializer * serializer, Utopia::instantiateAllExtensions< Serializer >())
            {
                this->setMessage(QString("  Registering %1 serializer").arg(serializer->description()));
                QSet< FileFormat * > formats = serializer->formats();
                QSetIterator< FileFormat * > i(formats);
                while (i.hasNext())
                {
                    FileFormat* format = i.next();
                    if (Serializer::get(format))
                    {
                        // FIXME Format already registered
                    }
                    else
                    {
                        Serializer::registerFormat(format, serializer);
                    }
                }
            }
        }

        void final()
        {}

        QString description()
        {
            return "Serializer initializer";
        }

    }; // class ParserInitializer

} // namespace Utopia




// dlsym handles

extern "C" const char * utopia_apiVersion()
{
    return UTOPIA_EXTENSION_LIBRARY_VERSION;
}

extern "C" const char * utopia_description()
{
    return "Utopia system initialisation";
}

extern "C" void utopia_registerExtensions()
{
    UTOPIA_REGISTER_EXTENSION(Utopia::SystemInitializer);
    UTOPIA_REGISTER_EXTENSION(Utopia::ParserInitializer);
    UTOPIA_REGISTER_EXTENSION(Utopia::SerializerInitializer);
}
