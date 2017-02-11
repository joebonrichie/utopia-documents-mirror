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

#include <utopia2/ontology.h>
#include <utopia2/node.h>

#include <QRegExp>

namespace Utopia
{

    //
    // class Ontology
    //

    /** Construct ontology object. */
    Ontology::Ontology(QString uri_)
        : _uri(uri_)
    {}

    /** Construct ontology object. */
    Ontology::Ontology(Node* node_)
        : _uri("")
    {
        if (node_ && node_->attributes.exists(UtopiaSystem.uri))
        {
            _uri = node_->attributes.get(UtopiaSystem.uri).toString();
        }
    }

    /** Assignment of thing. */
    Ontology& Ontology::operator = (QString uri_)
    {
        _uri = uri_;
        return *this;
    }

    /** Accessor dereference. */
    Node* Ontology::operator -> ()
    {
        return authority();
    }

    /** Accessor const dereference. */
    const Node* Ontology::operator -> () const
    {
        return authority();
    }

    /** Validity. */
    Ontology::operator bool ()
    {
        return !_uri.isEmpty();
    }

    /** Implicit cast dereference. */
    Ontology::operator Node* ()
    {
        return authority();
    }

    /** Implicit cast const dereference. */
    Ontology::operator const Node* () const
    {
        return authority();
    }

    /** Get ontology's authority (thing). */
    Node* Ontology::authority() const
    {
        return Node::getAuthority(_uri);
    }

    /** Get ontology's namespace. */
    QString Ontology::uri() const
    {
        return _uri;
    }

    /** Get term from Id. */
    Node* Ontology::term(const QString& id_) const
    {
        // Trivial case
        if (id_.isEmpty())
        {
            return authority();
        }

        // Full uri to search for

        QString uri = _uri + ((_uri.isEmpty() || _uri.at(_uri.length() - 1) == '/') ? "" : "#") + id_;
        // Not found by default
        Node* found = Node::getNode(uri);

        return found;
    }

    /** Does this ontology hold this Id. */
    bool Ontology::hasId(const QString& id_) const
    {
        return term(id_) != 0;
    }

    /** Does this ontology hold this Id. */
    bool Ontology::hasNode(Node* node_) const
    {
        return authority() && authority()->minions()->find(node_) != authority()->minions()->end();
    }

    /** Find ontology from namespace. */
    Ontology Ontology::fromURI(const QString& uri_, bool create_)
    {
        // Find (and possibly create) ontology thing
        Node* thing = Node::getAuthority(uri_, create_);

        return thing ? Ontology(thing) : Ontology();
    }

    /** Find ontology term from uri. */
    Node* Ontology::termFromURI(const QString& uri_)
    {
        QString ns;
        QString term;
        int lastDelimiter = uri_.lastIndexOf(QRegExp("[/#]"));
        if (lastDelimiter == -1)
        {
            term = uri_;
        }
        else
        {
            ns = uri_.left(lastDelimiter + ((uri_.at(lastDelimiter) == '/') ? 1 : 0));
            term = uri_.right(uri_.size() - lastDelimiter);
            qDebug() << uri_ << ns << term;
        }

        return Ontology::fromURI(ns).term(term);
    }



    //
    // Global ontology instantiations
    //

    RDFOntology rdf;
    RDFSOntology rdfs;
    SystemOntology UtopiaSystem;
    DomainOntology UtopiaDomain;
    MyGridOntology myGrid;
    OWLOntology owl;

} /* namespace Utopia */
