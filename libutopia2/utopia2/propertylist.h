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

#ifndef Utopia_PROPERTYLIST_H
#define Utopia_PROPERTYLIST_H

#include <utopia2/config.h>
#include <utopia2/list.h>
#include <utopia2/property.h>

#include <QSet>

namespace Utopia
{

    // Forward declarations
    class Node;

    /**
     *  \class _PropertyList
     *  \brief Concatenates a number of direct access lists.
     *
     *  Allows iteration over an arbitrary set of direct access lists.
     */
    class LIBUTOPIA_API _PropertyList
    {
    public:

        /** \name Construction and destruction. */
        //@{

        // Constructor
        _PropertyList(Node* node_, const Property& property_);

        //@}
        /** \name Iterators. */
        //@{

        class LIBUTOPIA_API iterator
        {
        public:
            // Iterator traits
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef Node* value_type;
            typedef size_t difference_type;
            typedef value_type& reference;
            typedef value_type* pointer;

            // Constructors
            iterator();
            iterator(Node* node_, const Property& property_, List* list_, List::iterator listIterator_, bool recursive_ = false);
            iterator(const iterator& rhs_);
            ~iterator();

            // Iterator operators
            iterator& operator = (const iterator& rhs_);
            bool operator == (const iterator& rhs_) const;
            bool operator != (const iterator& rhs_) const;
            reference operator * () const;
            iterator& operator ++ ();
            iterator operator ++ (int);
            iterator& operator -- ();
            iterator operator -- (int);

        private:
            // Node to which this Property List belongs
            Node* _node;
            // Property used in its instatiation
            Property _property;
            // Current List for that Property
            List* _list;
            // Current Node in that List
            List::iterator* _listIterator;

            // Recursion
            bool _recursive;
            iterator* _recursiveIterator;

        }; /* class iterator */

        class LIBUTOPIA_API const_iterator : public iterator
        {
        public:
            typedef const Node* value_type;
            typedef value_type& reference;
            typedef value_type* pointer;

            // Constructors
            const_iterator();
            const_iterator(Node* node_, const Property& property_, List* list_, List::iterator listIterator_, bool recursive_ = false);
            /**
             *  \brief Templated copy constructor, to allow both const and non-const copying.
             */
            template< typename _IteratorType > const_iterator(const _IteratorType& rhs_) : iterator(rhs_) {}

            // Iterator operators
            const_iterator& operator = (const iterator& rhs_);
            reference operator * () const;

        }; /* class const_iterator */

        // Typedefs for reverse iterators
        typedef std::reverse_iterator< iterator > reverse_iterator;
        typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

        //@}
        /** \name Iterator methods. */
        //@{

        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        iterator find(Node* node_);
        const_iterator find(Node* node_) const;
        reverse_iterator rbegin();
        const_reverse_iterator rbegin() const;
        reverse_iterator rend();
        const_reverse_iterator rend() const;

        //@}
        /** \name Collection methods. */
        //@{

        void append(Node* node_);
        Node* at(size_t index_);
//             Node* const at(size_t index_) const;
        Node* back();
        Node* const back() const;
        void clear();
        bool empty() const;
        bool exists(Node* node_) const;
//             iterator erase(Node* node_);
//             iterator erase(iterator start_, iterator end_);
//             iterator find(Node* node_);
//             const_iterator const find(Node* node_) const;
        Node* front();
        Node* const front() const;
//             int indexOf(Node* node_) const;
//             iterator insert(iterator loc_, Node* node_);
//             /**
//              *  \brief insert a given List of children at a given location.
//              *  \param start_ iterator to the first child to erase.
//              *  \param end_ iterator to the child after the last child to erase.
//              *  \return iterator to the child after the last erased child.
//              */
//             template< class input_iterator > void insert(iterator loc_, input_iterator start_, input_iterator end_)
//             {
//                 // iterate over the incoming nodes
//                 while (start_ != end_)
//                 {
//                     insert(loc_, *start_);
//                     ++start_;
//                 }
//             }
        Node* remove(Node* node_);
        size_t size() const;

        //@}

    private:
        // Node
        Node* _node;
        // Constituent properties
        Property _property;
        // List to search
        List* _list;

    }; // class _PropertyList

} // namespace Utopia

#endif // Utopia_PROPERTYLIST_H
