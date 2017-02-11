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

#ifndef Utopia_ONTOLOGY_H
#define Utopia_ONTOLOGY_H

#include <utopia2/config.h>
#include <utopia2/property.h>

#include <QString>

namespace Utopia
{

    class LIBUTOPIA_API Ontology
    {
    public:
        // Constructors
        Ontology(QString uri_);
        Ontology(Node* node_ = 0);

        // Assignment methods
        Ontology& operator = (QString uri_);

        // Accessor methods
        Node* operator -> ();
        const Node* operator -> () const;
        operator bool ();
        operator Node* ();
        operator const Node* () const;
        Node* authority() const;
        QString uri() const;
        Node* term(const QString& id_) const;

        // Query methods
        bool hasId(const QString& id_) const;
        bool hasNode(Node* node_) const;

        // Static factory methods
        static Ontology fromURI(const QString& uri_, bool create_ = false);
        static Node* termFromURI(const QString& uri_);

    private:
        // This ontology's namespace
        QString _uri;
        // This ontology's authority
        Node* _thing;

    }; /* class Ontology */

    class LIBUTOPIA_API RDFOntology : public Ontology
    {
    public:
        // RDF terms
        Property type;
        Node* property;
        Node* Statement;
        Property subject;
        Property predicate;
        Property object;

        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    }; /* class RDFOntology */
    LIBUTOPIA_API extern RDFOntology rdf;

    LIBUTOPIA_API extern class LIBUTOPIA_API RDFSOntology : public Ontology
    {
    public:
        // RDFS terms
        Node* Resource;
        Node* Class;
        Property subClassOf;
        Property subPropertyOf;
        Property seeAlso;
        Property isDefinedBy;
        Property comment;
        Property label;
        Node* Literal;
        Node* ConstraintResource;
        Node* ConstraintProperty;
        Property range;
        Property domain;
        Node* ContainerMembershipProperty;

        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    } rdfs; /* class RDFSOntology */

    LIBUTOPIA_API extern class LIBUTOPIA_API SystemOntology : public Ontology
    {
    public:
        // Utopia SYSTEM terms
        Node* Authority;
        Property uri;
        Property has;
        Property hasInput;
        Property hasAction;
        Property hasOutput;
        Property hasPart;
        Property hasAuthority;
        Property annotates;

        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    } UtopiaSystem; /* class SystemOntology */

    class LIBUTOPIA_API DomainOntology : public Ontology
    {
    public:
        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    }; /* class DomainOntology */
    LIBUTOPIA_API extern DomainOntology UtopiaDomain;

    LIBUTOPIA_API extern class LIBUTOPIA_API MyGridOntology : public Ontology
    {
    public:
        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    } myGrid; /* class MyGridOntology */

    LIBUTOPIA_API extern class LIBUTOPIA_API OWLOntology : public Ontology
    {
    public:
        using Ontology::operator =;
        using Ontology::operator ->;
        using Ontology::operator Node*;
        using Ontology::operator const Node*;

    } owl; /* class OWLOntology */

} /* namespace Utopia */

#endif // Utopia_ONTOLOGY_H
