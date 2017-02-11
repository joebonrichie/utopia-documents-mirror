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

#ifndef Utopia_LIST_H
#define Utopia_LIST_H

#include <utopia2/config.h>
#include <iterator>

namespace Utopia
{

    // Forward declarations
    class Node;
    class ListNode;

    /**
     *  \class List
     *
     *  This class implements a subset of the STL standard vector API, with a
     *  number of additional "direct access" methods.
     */
    class LIBUTOPIA_API List
    {
    public:
        // Iterators
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
            iterator(ListNode* last_, ListNode* cursor_);
            iterator(const iterator& rhs_);

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
            // Iterator state
            ListNode* _last;
            ListNode* _cursor;
        };

        class LIBUTOPIA_API const_iterator : public iterator
        {
        public:
            typedef Node* const value_type;
            typedef value_type& reference;
            typedef value_type* pointer;

            // Constructors
            const_iterator(ListNode* last_, ListNode* cursor_);
            /**
             *  \brief Templated copy constructor, to allow both const and non-const copying.
             */
            template< typename _IteratorType > const_iterator(const _IteratorType& rhs_) : iterator(rhs_) {}

            // Iterator operators
            const_iterator& operator = (const iterator& rhs_);
            reference operator * () const;
            const_iterator& operator ++ ();
            const_iterator operator ++ (int);
            const_iterator& operator -- ();
            const_iterator operator -- (int);

        }; /* class const_iterator */

        // Typedefs for reverse iterators
        typedef std::reverse_iterator< iterator > reverse_iterator;
        typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

        /** \name Construction and destruction methods. */
        //@{

        List();
        List(size_t capacity_);
        ~List();

        //@}
        /** \name Container methods. */
        //@{

        template< typename input_iterator > void assign(input_iterator from_, input_iterator to_)
        {
            this->clear();
            while (from_ != to_)
            {
                this->push_back(*from_);
                ++from_;
            }
        }
        Node* back();
        const Node* back() const;
        void clear();
        bool empty() const;
        iterator erase(iterator loc_);
        iterator erase(iterator begin_, iterator end_);
        bool exists(Node* node_) const;
        Node* front();
        const Node* front() const;
        iterator insert(iterator loc_, Node* node_);
        template< typename input_iterator > void insert(iterator loc_, input_iterator from_, input_iterator to_)
        {
            while (from_ != to_)
            {
                loc_ = ++this->insert(loc_, *from_++);
            }
        }
        void pop_back();
        void pop_front();
        void push_back(Node* node_);
        void push_front(Node* node_);
        void remove(Node* node_);
        size_t size() const;

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
        reverse_iterator rfind(Node* node_);
        const_reverse_iterator rfind(Node* node_) const;

        //@}

    private:
        // Array of List nodes
        ListNode* _data;
        // Capacity of List
        size_t _capacity;
        // Head and tail of List
        ListNode* _head;
        ListNode* _tail;
        // Size of List
        size_t _size;

        // New Node
        ListNode* _new(Node* node_);
        // Resize List
        void _resize();
        // Resolve Node
        ListNode* _resolve(Node* node_) const;

    }; // class List

} // namespace Utopia

#endif // Utopia_LIST_H
