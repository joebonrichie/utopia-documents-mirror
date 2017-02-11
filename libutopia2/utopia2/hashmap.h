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

#ifndef Utopia_HASHMAP_H
#define Utopia_HASHMAP_H

#include <utopia2/config.h>
#include <iterator>
#include <utility>

#include <QPair>
#include <QtDebug>


#ifdef DEBUG
#include <iostream>
#endif

namespace Utopia
{
    template< typename KeyType >
    class LIBUTOPIA_API hash
    {
    public:
        // Hash function
        unsigned long operator () (const KeyType& key_)
        {
            return key_.hash();
        }
    };

    template< typename ConcreteKeyType >
    class LIBUTOPIA_API hash< ConcreteKeyType* >
    {
    public:
        // Hash function
        unsigned long long operator () (ConcreteKeyType* key_)
        {
            return (unsigned long long) key_ >> 3;
        }
    };

    /**
     *  \class HashMap
     *
     *  This class implements a subset of the STL standard vector API, with a
     *  number of additional "direct access" methods.
     */
    template< typename KeyType, typename ValueType, size_t MAX_FAULT = 5 >
    class LIBUTOPIA_API HashMap
    {
        // Convenience
        typedef QPair< KeyType, ValueType* > ItemType;

    public:
        // Convenience
        typedef KeyType key_type;
        typedef ValueType* value_type;

        // Iterators
        class LIBUTOPIA_API iterator
        {
        public:
            // Iterator traits
            typedef std::forward_iterator_tag iterator_category;
            typedef ItemType value_type;
            typedef value_type& reference;
            typedef value_type* pointer;

            /**
             *  \brief Iterator constructor.
             *  \param head_ initial item of HashMap.
             *  \param tail_ item past the end of HashMap.
             *  \param cursor_ initial item to point at.
             */
            iterator(ItemType* head_, ItemType* tail_, ItemType* cursor_)
                : _head(head_), _tail(tail_), _cursor(cursor_)
            {
                // Skip to next valid item
                while (this->_cursor < tail_ && this->_cursor->second == 0) { ++this->_cursor; }
            }

            /**
             *  \brief Iterator copy constructor.
             *  \param rhs_ iterator to copy.
             */
            iterator(const iterator& rhs_)
            {
                this->operator = (rhs_);
            }

            /**
             *  \brief Assignment operator for an iterator.
             *  \param rhs_ source of assignment.
             *  \return self.
             */
            iterator& operator = (const iterator& rhs_)
                {
                    this->_head = rhs_._head;
                    this->_tail = rhs_._tail;
                    this->_cursor = rhs_._cursor;
                    return *this;
                }

            /**
             *  \brief Equality operator for an iterator.
             *  \param rhs_ iterator for which equality is being tested.
             *  \return equality.
             */
            bool operator == (const iterator& rhs_) const
            {
                return this->_cursor == rhs_._cursor;
            }

            /**
             *  \brief Inequality operator for an iterator.
             *  \param rhs_ iterator for which inequality is being tested.
             *  \return inequality.
             */
            bool operator != (const iterator& rhs_) const
            {
                return !(*this == rhs_);
            }

            /**
             *  \brief Dereference operator for an iterator.
             *  \return Dereferenced Node.
             */
            reference operator * () const
            {
                return *this->_cursor;
            }

            /**
             *  \brief Dereference operator for an iterator.
             *  \return Dereferenced Node.
             */
            pointer operator -> () const
            {
                return this->_cursor;
            }

            /**
             *  \brief Pre-increment operator for an iterator.
             *  \return self.
             */
            iterator& operator ++ ()
            {
                // Skip to next item
                while (++this->_cursor < this->_tail && this->_cursor->second == 0) {}

                return *this;
            }

            /**
             *  \brief Post-increment operator for an iterator.
             *  \return new iterator.
             */
            iterator operator ++ (int)
            {
                iterator tmp(*this);
                ++*this;
                return tmp;
            }

            /**
             *  \brief Pre-decrement operator for an iterator.
             *  \return self.
             */
            iterator& operator -- ()
            {
                // Skip to previous item
                while (--this->_cursor > this->_head && this->_cursor->second == 0) {}

                return *this;
            }

            /**
             *  \brief Post-decrement operator for an iterator.
             *  \return new iterator.
             */
            iterator operator -- (int)
            {
                iterator tmp(*this);
                --*this;
                return tmp;
            }

        private:
            // Iterator state
            ItemType* _head;
            ItemType* _tail;
            ItemType* _cursor;
        };

        class LIBUTOPIA_API const_iterator : public iterator
        {
        public:
            typedef const ItemType value_type;
            typedef value_type& reference;
            typedef value_type* pointer;

            /**
             *  \brief Iterator constructor.
             *  \param head_ initial item of HashMap.
             *  \param tail_ item past the end of HashMap.
             *  \param cursor_ initial item to point at.
             */
            const_iterator(ItemType* head_, ItemType* tail_, ItemType* cursor_)
                : iterator(head_, tail_, cursor_)
            {}

            /**
             *  \brief Templated copy constructor, to allow both const and non-const copying.
             */
            template< typename _IteratorType > const_iterator(const _IteratorType& rhs_)
                : iterator(rhs_)
            {}

            /**
             *  \brief Assignment operator for an const_iterator.
             *  \param rhs_ source of assignment.
             */
            const_iterator& operator = (const iterator& rhs_)
                {
                    this->iterator::operator = (rhs_);
                    return *this;
                }

            /**
             *  \brief Dereference operator for an const_iterator.
             *  \return Dereferenced Node.
             */
            reference operator * () const
            {
                return (reference) this->iterator::operator * ();
            }

            /**
             *  \brief Dereference operator for an iterator.
             *  \return Dereferenced Node.
             */
            pointer operator -> () const
            {
                return (pointer) this->iterator::operator -> ();
            }

            /**
             *  \brief Pre-increment operator for an const_iterator.
             *  \return self.
             */
            const_iterator& operator ++ ()
            {
                return (const_iterator&) this->iterator::operator ++ ();
            }

            /**
             *  \brief Post-increment operator for an const_iterator.
             *  \return new const_iterator.
             */
            const_iterator operator ++ (int)
            {
                return this->iterator::operator ++ (0);
            }

            /**
             *  \brief Pre-decrement operator for an const_iterator.
             *  \return self.
             */
            const_iterator& operator -- ()
            {
                return (const_iterator&) this->iterator::operator -- ();
            }

            /**
             *  \brief Post-decrement operator for an const_iterator.
             *  \return new const_iterator.
             */
            const_iterator operator -- (int)
            {
                return this->iterator::operator -- (0);
            }

        }; /* class const_iterator */

        // Typedefs for reverse iterators
        typedef std::reverse_iterator< iterator > reverse_iterator;
        typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

        /** \name Construction and destruction methods. */
        //@{

        /**
         *  \brief Default constructor of HashMap.
         */
        HashMap()
            : _capacity(1), _size(0)
        {
            // Create HashMap data
            this->_data = new ItemType[this->_capacity + MAX_FAULT];
            this->clear();
        }

        /**
         *  \brief Constructor of HashMap.
         *  \param capacity_ initial capacity of HashMap.
         */
        HashMap(size_t capacity_)
            : _capacity(capacity_), _size(0)
        {
            // Create HashMap data
            this->_data = new ItemType[this->_capacity + MAX_FAULT];
            this->clear();
        }

        /**
         *  \brief Destructor of HashMap.
         */
        ~HashMap()
        {
            delete [] this->_data;
        }

        //@}
        /** \name Container methods. */
        //@{

        /**
         *  \brief Assign a range of items.
         *  \param from_ start of range.
         *  \param to_ end of range.
         */
        template< typename input_iterator > void assign(input_iterator from_, input_iterator to_)
        {
            this->clear();
            while (from_ != to_)
            {
                this->insert(*from_++);
            }
        }

        /**
         *  \brief Clear HashMap.
         */
        void clear()
        {
            ::memset(this->_data, 0, sizeof(ItemType) * (this->_capacity + MAX_FAULT));
            this->_size = 0;
        }

        /**
         *  \brief Query emptiness.
         *  \return emptiness.
         */
        bool empty() const
        {
            return this->_size == 0;
        }

        /**
         *  \brief Erase an item at a given location.
         *  \param loc_ location of item to erase.
         */
        void erase(iterator loc_)
        {
            loc_->second = 0;
            --this->_size;
        }

        /**
         *  \brief Erase a range of items.
         *  \param from_ start of range.
         *  \param to_ end of range.
         */
        void erase(iterator begin_, iterator end_)
        {
            while (begin_ != end_)
            {
                this->erase(begin_++);
            }
        }

        /**
         *  \brief Erase an item that has a given key.
         *  \param key_ key of item to erase.
         */
        void erase(const KeyType& key_)
        {
            ItemType* cursor = this->_resolve(key_);
            if (cursor && cursor->second)
            {
                cursor->second = 0;
                --this->_size;
            }
        }

        /**
         *  \brief Check the existence of a particular key.
         *  \param key_ key to check for.
         *  \return existence.
         */
        bool exists(const KeyType& key_) const
        {
            ItemType* cursor = this->_resolve(key_);
            return cursor != 0 && cursor->second;                     }

        /**
         *  \brief Insert an item.
         *  \param item_ item to insert.
         *  \return iterator to inserted item, and whether an insertion took place.
         */
        std::pair< iterator, bool > insert(const ItemType& item_)
        {
            ItemType* cursor = this->_new(item_.first);
            bool insertion = cursor->second == 0;
            if (insertion) { cursor->first = item_.first; }
            cursor->second = item_.second;
            ++this->_size;
            return std::make_pair(iterator(this->_data, this->_data + this->_capacity + MAX_FAULT, cursor), insertion);
        }

        /**
         *  \brief Insert a range of items.
         *  \param from_ start of range.
         *  \param to_ end of range.
         */
        template< typename input_iterator > void insert(input_iterator from_, input_iterator to_)
        {
            while (from_ != to_)
            {
                this->insert(*from_++);
            }
        }

        /**
         *  \brief Return size of HashMap.
         *  \return size of HashMap.
         */
        size_t size() const
        {
            return this->_size;
        }

        /**
         *  \brief Dereference a value on a given key.
         *  \param key_ key to dereference.
         *  \return reference to the associated value.
         */
        ValueType*& operator [] (const KeyType& key_)
        {
            ItemType* cursor = this->_new(key_);
            if (cursor->second == 0)
            {
                cursor->first = key_;
                ++this->_size;
            }
            return cursor->second;
        }

        //@}
        /** \name Iterator methods. */
        //@{

        /**
         *  \brief Get an iterator to the beginning of the HashMap.
         *  \return requested iterator.
         */
        iterator begin()
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            return iterator(this->_data, tail, this->_data);
        }

        /**
         *  \brief Get an iterator to the beginning of the HashMap.
         *  \return requested iterator.
         */
        const_iterator begin() const
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            return iterator(this->_data, tail, this->_data);
        }

        /**
         *  \brief Get an iterator to the end of the HashMap.
         *  \return requested iterator.
         */
        iterator end()
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            return iterator(this->_data, tail, tail);
        }

        /**
         *  \brief Get an iterator to the end of the HashMap.
         *  \return requested iterator.
         */
        const_iterator end() const
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            return iterator(this->_data, tail, tail);
        }

        /**
         *  \brief Get iterator for a given Node.
         *  \param key_ Node to find.
         *  \return iterator to Node, or end() otherwise.
         */
        iterator find(KeyType key_)
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            ItemType* cursor = this->_resolve(key_);
            if (cursor == 0) { cursor = tail; }
            return iterator(this->_data, tail, cursor);
        }

        /**
         *  \brief Get iterator for a given Node.
         *  \param key_ Node to find.
         *  \return iterator to Node, or end() otherwise.
         */
        const_iterator find(KeyType key_) const
        {
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
            ItemType* cursor = this->_resolve(key_);
            if (cursor == 0) { cursor = tail; }
            return const_iterator(this->_data, tail, cursor);
        }

        /**
         *  \brief Get an iterator to the end of the HashMap.
         *  \return requested iterator.
         */
        reverse_iterator rbegin()
        {
            return reverse_iterator(this->end());
        }

        /**
         *  \brief Get an iterator to the end of the HashMap.
         *  \return requested iterator.
         */
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(this->end());
        }

        /**
         *  \brief Get an iterator to the beginning of the HashMap.
         *  \return requested iterator.
         */
        reverse_iterator rend()
        {
            return reverse_iterator(this->begin());
        }

        /**
         *  \brief Get an iterator to the beginning of the HashMap.
         *  \return requested iterator.
         */
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(this->begin());
        }

        //@}

    private:
        // Array of HashMap nodes
        ItemType* _data;
        // Capacity of HashMap
        size_t _capacity;
        // Size of HashMap
        size_t _size;

        /**
         *  \brief Initialise new ItemType for Node.
         */
        ItemType* _new(const KeyType& key_)
        {
            size_t index = hash< KeyType >()(key_) % this->_capacity;
            ItemType* cursor = this->_data + index;
            ItemType* empty = 0;
            int fault = MAX_FAULT;
            while (cursor->first != key_ && fault > 0)
            {
                if (cursor->second == 0 && empty == 0)
                {
                    empty = cursor;
                }
                ++cursor;
                --fault;
            }
            if (fault == 0)
            {
                if (empty == 0)
                {
                    this->_resize();
                    return this->_new(key_);
                }
                else
                {
                    return empty;
                }
            }
            else
            {
                return cursor;
            }
        }

        /**
         *  \brief Resize capacity of this HashMap.
         */
        void _resize()
        {
            // Get a handle to the old head
            ItemType* tail = this->_data + this->_capacity + MAX_FAULT;
#ifdef DEBUG
//                 qDebug() << "Resizing HashMap, capacity before:" << this->_capacity << "=>";
#endif
            // Increase capacity
            this->_capacity = 2 * this->_capacity + 1;
#ifdef DEBUG
//                 qDebug() << "  capacity after:"  << this->_capacity;
#endif
            // Get a handle to the old data
            ItemType* oldData = this->_data;
            // Get a handle to the old head
            ItemType* cursor = this->_data;
            // Create HashMap data
            this->_data = new ItemType[this->_capacity + MAX_FAULT];
            // Reset pointers
            this->_size = 0;
            // Transfer data to new memory
            while (cursor < tail)
            {
                if (cursor->second)
                {
                    this->insert(*cursor);
                }
                ++cursor;
            }
            // Delete old memory
            delete [] oldData;
        }

        /**
         *  \brief Resolve Node to ItemType.
         */
        ItemType* _resolve(const KeyType& key_) const
        {
            size_t index = hash< KeyType >()(key_) % this->_capacity;
            ItemType* cursor = this->_data + index;
            int fault = MAX_FAULT;
            while (cursor->first != key_ && fault > 0)
            {
                ++cursor;
                --fault;
            }
            return fault > 0 ? cursor : 0;
        }

    }; // class HashMap

} // namespace Utopia

#endif // Utopia_HASHMAP_H
