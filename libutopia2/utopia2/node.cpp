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

#include <utopia2/node.h>
#include <utopia2/list.h>
#include <utopia2/initializer.h>
#include <utopia2/ontology.h>

#include <QtDebug>

namespace Utopia
{

    // Constructor
    Node::Registry::Registry()
        : _initialised(false)
    {}
    // Destructor for cleanup
    Node::Registry::~Registry()
    {
        while (!_authorities.empty())
        {
            Node* doomed = *_authorities.begin();
            delete doomed;
        }
    }
    // Init
    void Node::Registry::init()
    {
        // Init!
        _initialised = true;

        // Bootstrap Utopia System ontology
        Node* _UtopiaSystem = new Node(true);
        UtopiaSystem.uri = new Node;
        UtopiaSystem.uri->setAuthority(_UtopiaSystem);
        _UtopiaSystem->attributes.set(UtopiaSystem.uri, "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system");
        UtopiaSystem = _UtopiaSystem;
        UtopiaSystem.Authority = new Node;
        UtopiaSystem.Authority->setAuthority(_UtopiaSystem);
        UtopiaSystem.Authority->attributes.set(UtopiaSystem.uri, "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system#Authority");
        UtopiaSystem.hasAuthority = new Node;
        UtopiaSystem.hasAuthority->setAuthority(_UtopiaSystem);
        UtopiaSystem.hasAuthority->attributes.set(UtopiaSystem.uri, "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system#hasAuthority");
        UtopiaSystem.hasPart = new Node;
        UtopiaSystem.hasPart->setAuthority(_UtopiaSystem);
        UtopiaSystem.hasPart->attributes.set(UtopiaSystem.uri, "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system#hasPart");
        UtopiaSystem.uri->attributes.set(UtopiaSystem.uri, "http://utopia.cs.manchester.ac.uk/2007/03/utopia-system#uri");

        // Bootstrap RDF ontology
        Node* _rdf = new Node(true);
        _rdf->attributes.set(UtopiaSystem.uri, "http://www.w3.org/1999/02/22-rdf-syntax-ns");
        rdf = _rdf;
        rdf.type = new Node;
        rdf.type->setAuthority(_rdf);
        rdf.type->attributes.set(UtopiaSystem.uri, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
        rdf.property = new Node;
        rdf.property->setAuthority(_rdf);
        rdf.property->attributes.set(UtopiaSystem.uri, "http://www.w3.org/1999/02/22-rdf-syntax-ns#Property");

        // Bootstrap RDFS ontology
        Node* _rdfs = new Node(true);
        _rdfs->attributes.set(UtopiaSystem.uri, "http://www.w3.org/2000/01/rdf-schema");
        rdfs = _rdfs;
        rdfs.Class = new Node;
        rdfs.Class->setAuthority(_rdfs);
        rdfs.Class->attributes.set(UtopiaSystem.uri, "http://www.w3.org/2000/01/rdf-schema#Class");
        rdfs.subClassOf = new Node;
        rdfs.subClassOf->setAuthority(_rdfs);
        rdfs.subClassOf->attributes.set(UtopiaSystem.uri, "http://www.w3.org/2000/01/rdf-schema#subClassOf");
        rdfs.subPropertyOf = new Node;
        rdfs.subPropertyOf->setAuthority(_rdfs);
        rdfs.subPropertyOf->attributes.set(UtopiaSystem.uri, "http://www.w3.org/2000/01/rdf-schema#subPropertyOf");
    }

    // Get root
    QSet< Node* >& Node::Registry::authorities()
    {
        return get()._authorities;
    }
    // Get Node with URI
    QMap< QString, Node* >& Node::Registry::uris()
    {
        return get()._uris;
    }

    // Remove Node from URI map
    void Node::Registry::removeUri(Node* node_)
    {
        uris().remove(node_->attributes.get(UtopiaSystem.uri).toString());
    }
    // Add Node to URI map
    void Node::Registry::addUri(Node* node_)
    {
        uris()[node_->attributes.get(UtopiaSystem.uri).toString()] = node_;
    }

    // Static get
    Node::Registry& Node::Registry::get()
    {
        static Registry reg;
        if (!reg._initialised)
        {
            reg.init();
        }
        return reg;
    }

    /** Factory method for creating an authority Node. */
    Node* createAuthority(Node* superAuthority_)
    {
        Node* new_node = new Node(true);
        new_node->setType(UtopiaSystem.Authority);
        new_node->setAuthority(superAuthority_);
        return new_node;
    }

    /** Factory method for creating a class Node. */
    Node* createClass(Node* authority_, Node* superClass_)
    {
        Node* new_node = authority_->create(rdfs.Class);
        if (superClass_)
        {
            new_node->relations(rdfs.subClassOf).append(superClass_ ? superClass_ : authority_);
        }
        return new_node;
    }

    /** Factory method for creating a Property Node. */
    Node* createProperty(Node* authority_, Node* superProperty_)
    {
        Node* new_node = authority_->create(rdf.property);
        if (superProperty_)
        {
            new_node->relations(rdfs.subPropertyOf).append(superProperty_);
        }
        return new_node;
    }

    /** Factory method for creating a Node. */
    Node* createNode(Node* authority_, Node* type_)
    {
        Node* new_node = new Node;
        new_node->setAuthority(authority_);
        new_node->setType(type_);
        return new_node;
    }

    /** Factory method for creating a Node. */
    Node* Node::create(Node* type_)
    {
        if (_minions)
        {
            return createNode(this, type_);
        }
        else
        {
            return createNode(authority(), type_);
        }
    }

    /** Factory method for creating a Node. */
    Node* Node::create(QString type_)
    {
        Node* found = Node::getNode(type_);

        if (!found)
        {
            // Create a new Node in the empty namespace
            found = getAuthority("", true);
            found->attributes.set(UtopiaSystem.uri, type_);
        }

        return this->create(found);
    }


    /** Constructor for Node. */
    Node::Node(bool authority_)
        : attributes(*this), relations(*this), _minions(0), _authority(0), _instances(0), _type(0)
    {
        if (authority_)
        {
            // Then add to Registry
            Registry::authorities().insert(this);
            // Set up minion List
            _minions = new List;
        }
    }

    /** Destructor for Node. */
    Node::~Node()
    {
        // Remove from authority
        setAuthority(0);
        // Remove from authority
        setType(0);

        // Has minions?
        if (_minions)
        {
            // Then remove from Registry
            Registry::authorities().remove(this);

            // Delete minions
            List::iterator minion_iter = _minions->begin();
            List::iterator minion_end = _minions->end();
            for (; minion_iter != minion_end; ++minion_iter)
            {
                (*minion_iter)->_authority = 0;
                delete *minion_iter;
            }
            delete _minions;
        }

        // Has instances?
        if (_instances)
        {
            // Delete instances
            while (!_instances->empty())
            {
                _instances->back()->setType(0);
            }
            delete _instances;
        }
    }

    // Add minion
    void Node::_addMinion(Node* node_)
    {
        if (_minions)
        {
            _minions->push_back(node_);
        }
    }

    // Remove minion
    void Node::_removeMinion(Node* node_)
    {
        if (_minions)
        {
            _minions->erase(_minions->find(node_));
        }
    }

    // Get minions
    List* Node::minions() const
    {
        return _minions;
    }

    // Set authority
    void Node::setAuthority(Node* authority_)
    {
        if (_authority)
        {
            _authority->_removeMinion(this);
        }
        _authority = authority_;
        if (_authority)
        {
            _authority->_addMinion(this);
        }
    }

    // Get authority
    Node* Node::authority() const
    {
        return _authority;
    }

    // Add instance
    void Node::_addInstance(Node* node_)
    {
        if (!_instances)
        {
            _instances = new List;
        }

        _instances->push_back(node_);
    }

    // Remove instance
    void Node::_removeInstance(Node* node_)
    {
        if (_instances)
        {
            _instances->erase(_instances->find(node_));
        }
    }

    // Get instances
    List* Node::instances() const
    {
        return _instances;
    }

    // Set type
    void Node::setType(Node* type_)
    {
        if (_type)
        {
            _type->_removeInstance(this);
        }
        _type = type_;
        if (_type)
        {
            _type->_addInstance(this);
        }
    }

    // Get type
    Node* Node::type() const
    {
        return _type;
    }

    /** Static authority retrieval. */
    Node* Node::getAuthority(QString uri_, bool create_)
    {
        QSet< Node* >::iterator found = Registry::authorities().begin();
        QSet< Node* >::iterator end = Registry::authorities().end();
        for (; found != end; ++found)
        {
            if ((*found)->attributes.exists(UtopiaSystem.uri) && (*found)->attributes.get(UtopiaSystem.uri).toString() == uri_)
            {
                return *found;
            }
        }

        if (create_)
        {
            Node* auth = createAuthority();
            auth->attributes.set(UtopiaSystem.uri, uri_);
            return auth;
        }
        else
        {
            return 0;
        }
    }

    /** Static Node retrieval. */
    Node* Node::getNode(QString uri_)
    {
        QMap< QString, Node* >::iterator found = Registry::uris().find(uri_);
        return found != Registry::uris().end() ? found.value() : 0;
    }



    //
    // class Node::attribution
    //

    /** Constructor for internal class. */
    Node::attribution::attribution(Node& node_)
        : _node(node_)
    {}

    /** Destructor for internal class. */
    Node::attribution::~attribution()
    {
        clear();
    }

    /**
     *  \brief Check for existance of a specific key.
     *  \param key_ Key to look for.
     */
    bool Node::attribution::exists(const QString& key_) const
    {
        return exists(fromURI(key_));
    }

    /**
     *  \brief Check for existance of a specific key.
     *  \param key_ Key to look for.
     */
    bool Node::attribution::exists(Node* key_) const
    {
        return _attributes.exists(key_);
    }

    /**
     *  \brief Remove a specific attribute.
     *  \param key_ Key of attribute to remove.
     */
    void Node::attribution::remove(const QString& key_)
    {
        return remove(fromURI(key_));
    }

    /**
     *  \brief Remove a specific attribute.
     *  \param key_ Key of attribute to remove.
     */
    void Node::attribution::remove(Node* key_)
    {
        // Delete old Variant object if one exists, then remove from map
        if (exists(key_))
        {
            // Unregister URI if so
            if (key_ == UtopiaSystem.uri)
            {
                Node::Registry::removeUri(&_node);
            }

            delete _attributes[key_];
            _attributes.erase(key_);
        }
    }

    /** Clear all attributes of this Node. */
    void Node::attribution::clear()
    {
        // Unregister from URI map
        if (exists(UtopiaSystem.uri))
        {
            Node::Registry::removeUri(&_node);
        }

        // Delete each Variant object
        AttributeMap::iterator iter = _attributes.begin();
        AttributeMap::iterator end = _attributes.end();
        for (; iter != end; ++iter)
        {
            delete iter->second;
        }
        _attributes.clear();
    }

    /** Count attributes of this Node. */
    size_t Node::attribution::count() const
    {
        return _attributes.size();
    }

    /** Return attribute keys. */
    QList< Node* > Node::attribution::keys() const
    {
        QList< Node* > keys;
        AttributeMap::const_iterator iter = _attributes.begin();
        AttributeMap::const_iterator end = _attributes.end();
        for (; iter != end; ++iter)
        {
            keys.push_back(iter->first);
        }
        return keys;
    }

    /** Get type of attribute. */
    QString Node::attribution::typeOf(const QString& key_) const
    {
        return typeOf(fromURI(key_));
    }

    /** Get type of attribute. */
    QString Node::attribution::typeOf(Node* key_) const
    {
        return exists(key_) ? _attributes.find(key_)->second->typeName() : "";
    }

    /** Static convenience method. */
    Node* Node::attribution::fromURI(QString uri_)
    {
        Node* found = Node::getNode(uri_);

        if (!found)
        {
            // Create a new Node in the empty namespace
            found = getAuthority("", true);
            found->attributes.set(UtopiaSystem.uri, uri_);
        }

        return found;
    }

    void Node::attribution::removeUri(Node* node_)
    {
        Registry::removeUri(node_);
    }

    void Node::attribution::addUri(Node* node_)
    {
        Registry::addUri(node_);
    }



    //
    // class Node::relation
    //

    QList< Property > Node::relation::operator () ()
    {
        QList< Property > list;
        RelationMap::iterator iter = this->_relations.begin();
        RelationMap::iterator end = this->_relations.end();
        for (; iter != end; ++iter)
        {
            if (iter->first.dir() == Forward)
            {
                list.append(iter->first);
            }
        }
        return list;
    }

    /**
     *  \brief Create a _PropertyList object for access to relations.
     */
    _PropertyList Node::relation::operator () (const Property& property_)
    {
        return _PropertyList(&_node, property_);
    }

    /**
     *  \brief Create a _PropertyList object for access to relations.
     */
    _PropertyList Node::relation::operator () (const Property& property_) const
    {
        return _PropertyList(&_node, property_);
    }

    /**
     *  \brief Resolve a Property into a direct access List.
     *
     *  This method is called internally as part of iterator management, you
     *  should never need to call this method yourself.
     */
    List* Node::relation::_getDirectAccessList(const Property& property_, bool create_)
    {
        RelationMap::iterator list_iter = _relations.find(property_);
        RelationMap::iterator list_end = _relations.end();
        if (list_iter != list_end)
        {
//             qDebug() << "old (" << property_ << ")" << list_iter->second;
            return list_iter->second;
        }
        else
        {
            List* ret = create_ ? (_relations[property_] = new List) : 0;
//             qDebug() << "new (" << property_ << ")  << ret;
            return ret;
//             return create_ ? (_relations[property_] = new List) : 0;
        }
    }

#ifdef DEBUG
    void Node::relation::print() const
    {
        qDebug() << &_node << "[";
        RelationMap::const_iterator list_iter = _relations.begin();
        RelationMap::const_iterator list_end = _relations.end();
        for (; list_iter != list_end; ++list_iter)
        {
            qDebug() << "    " << list_iter->first;
            List::iterator iter = list_iter->second->begin();
            while (iter != list_iter->second->end())
            {
                if (iter != list_iter->second->begin())
                {
                    qDebug() << ",";
                }
                if ((*iter)->attributes.exists(UtopiaSystem.uri))
                {
                    qDebug() << ((*iter)->attributes.get(UtopiaSystem.uri).toString());
                }
                else
                {
                    qDebug() << *iter;
                }
                ++iter;
            }
        }
        qDebug() << "]";
    }
#endif

    /** Constructor for internal class. */
    Node::relation::relation(Node& node_)
        : _node(node_)
    {}

    /** Destructor for internal class. */
    Node::relation::~relation()
    {
        RelationMap::const_iterator list_iter = _relations.begin();
        RelationMap::const_iterator list_end = _relations.end();
        for (; list_iter != list_end; ++list_iter)
        {
            delete list_iter->second;
        }
    }

    /** Is this Node unrelated to anything? */
    bool Node::relation::empty() const
    {
        return this->_relations.empty();
    }

} /* namespace Utopia */
