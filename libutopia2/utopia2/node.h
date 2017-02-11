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

#ifndef Utopia_NODE_H
#define Utopia_NODE_H

#include <utopia2/config.h>
#include <utopia2/ontology.h>
#include <utopia2/property.h>
#include <utopia2/propertylist.h>
#include <utopia2/global.h>
#include <utopia2/hashmap.h>

#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

namespace Utopia
{

    // Forwards
    class List;

    // Factory functions
    LIBUTOPIA_EXPORT Node* createAuthority(Node* superAuthority_ = 0);
    LIBUTOPIA_EXPORT Node* createClass(Node* authority_, Node* superClass_ = 0);
    LIBUTOPIA_EXPORT Node* createProperty(Node* authority_, Node* superProperty_ = 0);
    LIBUTOPIA_EXPORT Node* createNode(Node* authority_, Node* type_ = 0);

    /**
     *  \class Node
     *  \brief The generic Node class of the Utopia model.
     */
    class LIBUTOPIA_API Node
    {
    public:
        /** \name Constuction, destruction and factory methods. */
        //@{

        // Destructor
        ~Node();

        Node* create(Node* type_ = 0);
        Node* create(QString type_);

        //@}
        /** \name Attribute methods. */
        //@{

        /**
         *  \class attribution
         *  \brief The attribution helper class.
         */
        class LIBUTOPIA_API attribution
        {
        public:
            // Initialise attribution class
            attribution(Node& node_);
            ~attribution();

            // Is this attribute key present?
            bool exists(const QString& key_) const;
            bool exists(Node* key_) const;
            // Remove an attribute entirely
            void remove(const QString& key_);
            void remove(Node* key_);
            // Clear all attributes
            void clear();
            // Count attributes
            size_t count() const;
            // Return keys
            QList< Node* > keys() const;
            // Get type of attribute
            QString typeOf(const QString& key_) const;
            QString typeOf(Node* key_) const;

            /**
             *  \brief Get a particular attribute.
             *  \param key_ Key of attribute to retrieve.
             *
             *  Should always be used in conjunction with \a exists().
             */
            QVariant get(const QString& key_, QVariant default_ = QVariant())
            {
                // Delegate
                return get(fromURI(key_), default_);
            }

            /**
             *  \brief Get a particular attribute.
             *  \param key_ Key of attribute to retrieve.
             *
             *  Should always be used in conjunction with \a exists().
             */
            QVariant get(Node* key_, QVariant default_ = QVariant())
            {
                // Cannot use operator[] as there is no const version, so must use find().
                if (_attributes.exists(key_))
                {
                    QVariant * v = _attributes[key_];
                    //qDebug() << "***** get" << v;
                    return *v; // FIXME
                }
                else
                {
                    return default_;
                }
            }

            /**
             *  \brief Get a particular attribute.
             *  \param key_ Key of attribute to retrieve.
             *
             *  If \a key_ does not exist, behaviour is undefined, therefore
             *  should always be used in conjunction with \a exists().
             */
            const QVariant get(const QString& key_, QVariant default_ = QVariant()) const
            {
                // Delegate
                return get(fromURI(key_), default_);
            }

            /**
             *  \brief Get a particular attribute.
             *  \param key_ Key of attribute to retrieve.
             *
             *  If \a key_ does not exist, behaviour is undefined, therefore
             *  should always be used in conjunction with \a exists().
             */
            const QVariant get(Node* key_, QVariant default_ = QVariant()) const
            {
                // Cannot use operator[] as there is no const version, so must use find().
                if (_attributes.exists(key_))
                {
                    return *_attributes.find(key_)->second;
                }
                else
                {
                    return default_;
                }
            }

            /**
             *  \brief Set an attribute on this Node.
             *  \param key_ Key of attribute to set.
             *  \param value_ Value to set attribute to.
             *
             *  If \a key_ exists then its value is changed to \a value_, else
             *  a new key-value pair is created.
             */
            void set(const QString& key_, const QVariant& value_)
            {
                // Delegate
                set(fromURI(key_), value_);
            }

            /**
             *  \brief Set an attribute on this Node.
             *  \param key_ Key of attribute to set.
             *  \param value_ Value to set attribute to.
             *
             *  If \a key_ exists then its value is changed to \a value_, else
             *  a new key-value pair is created.
             */
            void set(Node* key_, const QVariant value_)
            {
                // Delete old Variant object if one exists
                if (exists(key_))
                {
                    // Unregister URI if so
                    if (key_ == UtopiaSystem.uri)
                    {
                        removeUri(&_node);
                    }

                    delete _attributes[key_];
                }

                // Create new Variant object
                QVariant * v = new QVariant(value_);
                //qDebug() << "***** set" << v;
                _attributes[key_] = v;

                // Register URI if so
                if (key_ == UtopiaSystem.uri)
                {
                    addUri(&_node);
                }
            }

        private:
            // Actual Node
            Node& _node;

            // Attribute map
            typedef HashMap< Node*, QVariant, 3 > AttributeMap;
            AttributeMap _attributes;

            // Friends
            friend class Node;

            // Convert a string into a Node
            static Node* fromURI(QString uri_);
            static void removeUri(Node* node_);
            static void addUri(Node* node_);

        } attributes;

        //@}
        /** \name Property methods. */
        //@{

        /**
         *  \class relation
         *  \brief The relation helper class.
         */
        class LIBUTOPIA_API relation
        {
        public:
            // Iterators
            typedef _PropertyList::iterator iterator;
            typedef _PropertyList::const_iterator const_iterator;
            typedef _PropertyList::reverse_iterator reverse_iterator;
            typedef _PropertyList::const_reverse_iterator const_reverse_iterator;

            // General methods on all properties
//                    void clear();
            bool empty() const;
//                    Node* remove(Node* node_);
//                    size_t size() const;

            // Association limiters
            QList< Property > operator () ();
            _PropertyList operator () (const Property& property_);
            _PropertyList operator () (const Property& property_) const;

            // Iterator methods
            //             iterator begin(criterion* criterion_ = 0);
            //             const_iterator begin(criterion* criterion_ = 0) const;
            //             iterator end();
            //             const_iterator end() const;

            // Reverse iterator methods
            //             reverse_iterator rbegin();
            //             const_reverse_iterator rbegin() const;
            //             reverse_iterator rend();
            //             const_reverse_iterator rend() const;

            // Convenience...
            List* _getDirectAccessList(const Property& property_, bool create_ = false);

#ifdef DEBUG
            // Debug
            void print() const;
#endif

        private:
            // Constructor
            relation(Node& node_);
            ~relation();

            // Reference to containing Node
            Node& _node;

            // Forward properties
            typedef HashMap< Property, List, 3 > RelationMap;
            RelationMap _relations;

            // Friends
            friend class _PropertyList;
            friend class Node;

        } relations;

        //@}

        // Get authority Node from Registry
        static Node* getAuthority(QString uri_, bool create_ = false);
        static Node* getNode(QString uri_);

        // Set authority
        void setAuthority(Node* authority_);
        // Get authority
        Node* authority() const;
        // Get minions
        List* minions() const;
        // Set type
        void setType(Node* type_);
        // Get type
        Node* type() const;
        // Get instances
        List* instances() const;

    private:
        // Has authority over...
        List* _minions;
        // Authority?
        Node* _authority;
        // Add minion
        void _addMinion(Node* node_);
        // Remove minion
        void _removeMinion(Node* node_);
        // Instances of this Node
        List* _instances;
        // Type
        Node* _type;
        // Add instance
        void _addInstance(Node* node_);
        // Remove instance
        void _removeInstance(Node* node_);

        // Private constructors
        Node(bool authority_ = false);

        friend class Initializer;
        friend Node* createAuthority(Node*);
        friend Node* createClass(Node*, Node*);
        friend Node* createProperty(Node*, Node*);
        friend Node* createNode(Node*, Node*);

        // Registry class
        class Registry
        {
        public:
            // Constructor
            Registry();
            // Destructor for cleanup
            ~Registry();
            // Init
            void init();

            // Get root
            static QSet< Node* >& authorities();
            // Get Node with URI
            static QMap< QString, Node* >& uris();

            // Remove Node from URI map
            static void removeUri(Node* node_);
            // Add Node to URI map
            static void addUri(Node* node_);

        private:
            // Authorities
            QSet< Node* > _authorities;
            // All URIs
            QMap< QString, Node* > _uris;
            // Initialised?
            bool _initialised;

            // Static get
            static Registry& get();

            friend class Node;

        }; /* class _static_registry */

        static Registry registryInstance;

    }; /* class Node */

} /* namespace Utopia */

Q_DECLARE_METATYPE(Utopia::Node*);

#endif /* Utopia_NODE_H */
