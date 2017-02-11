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

#include <utopia2/list.h>
#include <string.h>

#ifdef DEBUG
#include <iostream>
#include <QtDebug>
#endif

#define Utopia_MAX_FAULT 5

namespace Utopia
{
    // Hash a pointer...
    unsigned long hashOf(Node* node_)
    {
        return ((unsigned long long) node_) >> 4;
    }

    class ListNode
    {
    public:
        // Constructor
        ListNode();

        // Hash function
        unsigned long hash() const;

        // Data members
        ListNode* next;
        ListNode* prev;
        Node* data;
    };

    /**
     *  \brief Constructor of ListNode.
     */
    ListNode::ListNode()
        : next(0), prev(0), data(0)
    {}

    /**
     *  \brief Hash of List Node object.
     *  \param ListNode_ List Node to hash.
     *  \return hash of List Node.
     */
    unsigned long ListNode::hash() const
    {
        return hashOf(this->data);
    }

    /**
     *  \brief Default constructor of List.
     */
    List::List()
        : _capacity(1), _head(0), _tail(0), _size(0)
    {
        // Create List data
        this->_data = new ListNode[this->_capacity + Utopia_MAX_FAULT];
    }

    /**
     *  \brief Constructor of List.
     *  \param capacity_ initial capacity of List.
     */
    List::List(size_t capacity_)
        : _capacity(capacity_), _head(0), _tail(0), _size(0)
    {
        // Create List data
        this->_data = new ListNode[capacity_ + Utopia_MAX_FAULT];
    }

    /**
     *  \brief Destructor of List.
     */
    List::~List()
    {
        delete [] this->_data;
    }

    /**
     *  \brief Return Node from back of List.
     *  \return requested Node.
     */
    Node* List::back()
    {
        return this->_tail->data;
    }

    /**
     *  \brief Return Node from back of const List.
     *  \return requested Node.
     */
    const Node* List::back() const
    {
        return this->_tail->data;
    }

    /**
     *  \brief Clear List.
     */
    void List::clear()
    {
        ::memset(this->_data, 0, sizeof(ListNode) * (this->_capacity + Utopia_MAX_FAULT));
        this->_head = 0;
        this->_tail = 0;
        this->_size = 0;
    }

    /**
     *  \brief Query emptiness.
     *  \return emptiness.
     */
    bool List::empty() const
    {
        return this->_size == 0;
    }

    /**
     *  \brief Erase Node at a specific location.
     *  \param loc_ location of Node to erase.
     *  \return iterator to subsequent Node.
     */
    List::iterator List::erase(iterator loc_)
    {
        Node* doomed = *loc_;
        ++loc_;
        ListNode* tomb = this->_resolve(doomed);
        if (tomb->prev)
        {
            tomb->prev->next = tomb->next;
        }
        else
        {
            this->_head = tomb->next;
        }
        if (tomb->next)
        {
            tomb->next->prev = tomb->prev;
        }
        else
        {
            this->_tail = tomb->prev;
        }
        tomb->next = 0;
        tomb->prev = 0;
        tomb->data = 0;
        --this->_size;
        return loc_;
    }

    /**
     *  \brief Erase Node at a specific location.
     *  \param begin_ start of sublist to erase.
     *  \param end_ end of sublist to erase.
     *  \return iterator to subsequent Node.
     */
    List::iterator List::erase(iterator begin_, iterator end_)
    {
        Node* ldoomed = *begin_;
        Node* rdoomed = *end_;
        ListNode* tomb = this->_resolve(ldoomed);
        ListNode* rtomb = this->_resolve(rdoomed);
        while (tomb != rtomb)
        {
            if (tomb->prev)
            {
                tomb->prev->next = tomb->next;
            }
            else
            {
                this->_head = tomb->next;
            }
            if (tomb->next)
            {
                tomb->next->prev = tomb->prev;
            }
            else
            {
                this->_tail = tomb->prev;
            }
            ListNode* next = tomb->next;
            tomb->next = 0;
            tomb->prev = 0;
            tomb->data = 0;
            --this->_size;
            tomb = next;
        }
        return end_;
    }

    /**
     *  \brief Query the presence of a given Node.
     *  \return truth or otherwise of the Node's existence.
     */
    bool List::exists(Node* node_) const
    {
        return this->_resolve(node_) != 0;
    }

    /**
     *  \brief Return Node from front of List.
     *  \return requested Node.
     */
    Node* List::front()
    {
        return this->_head->data;
    }

    /**
     *  \brief Return Node from front of const List.
     *  \return requested Node.
     */
    const Node* List::front() const
    {
        return this->_head->data;
    }

    /**
     *  \brief Insert Node at given location.
     *  \param loc_ location to insert Node.
     *  \param node_ Node to insert.
     *  \return iterator to inserted Node.
     */
    List::iterator List::insert(iterator loc_, Node* node_)
    {
        // If we're appending this data then use push_back()
        if (loc_ == this->end())
        {
            this->push_back(node_);
            return --end();
        }
        // Else if we're prepending this data, then use push_front()
        else if (loc_ == this->begin())
        {
            this->push_front(node_);
            return begin();
        }
        // Otherwise...
        else
        {
            ListNode* cursor = this->_new(node_);
            // If duplicate, then return end()
            if (cursor->data)
            {
                return end();
            }
            // Otherwise add to middle of List at cursor
            else
            {
                ListNode* location = this->_resolve(*loc_);
                cursor->data = node_;
                cursor->next = location;
                cursor->prev = location->prev;
                cursor->prev->next = cursor;
                cursor->next->prev = cursor;
                return iterator(this->_tail, cursor);
            }
        }
    }

    /**
     *  \brief Pop the back of this List.
     */
    void List::pop_back()
    {
        // Remove data pointer
        this->_tail->data = 0;
        --this->_size;

        // Relink
        if (this->_tail->prev)
        {
            this->_tail->prev->next = 0;
            this->_tail = this->_tail->prev;
        }
        else
        {
            this->_tail = 0;
            this->_head = 0;
        }
    }

    /**
     *  \brief Pop the front of this List.
     */
    void List::pop_front()
    {
        // Remove data pointer
        this->_head->data = 0;
        --this->_size;

        // Relink
        if (this->_head->next)
        {
            this->_head->next->prev = 0;
            this->_head = this->_head->next;
        }
        else
        {
            this->_tail = 0;
            this->_head = 0;
        }
    }

    /**
     *  \brief Push to the back of this List the given Node.
     *  \param node_ Node to push.
     */
    void List::push_back(Node* node_)
    {
        ListNode* cursor = this->_new(node_);
        if (cursor->data == 0)
        {
            cursor->data = node_;
            cursor->prev = this->_tail;
            cursor->next = 0;
            if (this->_tail) { this->_tail->next = cursor; }
            if (this->_head == 0) { this->_head = cursor; }
            this->_tail = cursor;
            ++this->_size;
        }
    }

    /**
     *  \brief Push to the front of this List the given Node.
     *  \param node_ Node to push.
     */
    void List::push_front(Node* node_)
    {
        ListNode* cursor = this->_new(node_);
        if (cursor->data == 0)
        {
            cursor->data = node_;
            cursor->next = this->_head;
            cursor->prev = 0;
            if (this->_head) { this->_head->prev = cursor; }
            if (this->_tail == 0) { this->_tail = cursor; }
            this->_head = cursor;
            ++this->_size;
        }
    }

    /**
     *  \brief Remove a Node from a List.
     *  \param node_ Node to remove.
     */
    void List::remove(Node* node_)
    {
        this->erase(this->find(node_));
    }

    /**
     *  \brief Return size of List.
     *  \return size of List.
     */
    size_t List::size() const
    {
        return this->_size;
    }

    /**
     *  \brief Get an iterator to the beginning of the List.
     *  \return requested iterator.
     */
    List::iterator List::begin()
    {
        return iterator(this->_tail, this->_head);
    }

    /**
     *  \brief Get an iterator to the beginning of the List.
     *  \return requested iterator.
     */
    List::const_iterator List::begin() const
    {
        return iterator(this->_tail, this->_head);
    }

    /**
     *  \brief Get an iterator to the end of the List.
     *  \return requested iterator.
     */
    List::iterator List::end()
    {
        return iterator(this->_tail, 0);
    }

    /**
     *  \brief Get an iterator to the end of the List.
     *  \return requested iterator.
     */
    List::const_iterator List::end() const
    {
        return iterator(this->_tail, 0);
    }

    /**
     *  \brief Get iterator for a given Node.
     *  \param node_ Node to find.
     *  \return iterator to Node, or end() otherwise.
     */
    List::iterator List::find(Node* node_)
    {
        return iterator(this->_tail, this->_resolve(node_));
    }

    /**
     *  \brief Get iterator for a given Node.
     *  \param node_ Node to find.
     *  \return iterator to Node, or end() otherwise.
     */
    List::const_iterator List::find(Node* node_) const
    {
        return const_iterator(this->_tail, this->_resolve(node_));
    }

    /**
     *  \brief Get an iterator to the end of the List.
     *  \return requested iterator.
     */
    List::reverse_iterator List::rbegin()
    {
        return reverse_iterator(this->end());
    }

    /**
     *  \brief Get an iterator to the end of the List.
     *  \return requested iterator.
     */
    List::const_reverse_iterator List::rbegin() const
    {
        return const_reverse_iterator(this->end());
    }

    /**
     *  \brief Get an iterator to the beginning of the List.
     *  \return requested iterator.
     */
    List::reverse_iterator List::rend()
    {
        return reverse_iterator(this->begin());
    }

    /**
     *  \brief Get an iterator to the beginning of the List.
     *  \return requested iterator.
     */
    List::const_reverse_iterator List::rend() const
    {
        return const_reverse_iterator(this->begin());
    }

    /**
     *  \brief Get iterator for a given Node.
     *  \param node_ Node to find.
     *  \return iterator to Node, or rend() otherwise.
     */
    List::reverse_iterator List::rfind(Node* node_)
    {
        iterator found = this->find(node_);
        if (found == this->end())
        {
            return rend();
        }
        else
        {
            return reverse_iterator(++found);
        }
    }

    /**
     *  \brief Get iterator for a given Node.
     *  \param node_ Node to find.
     *  \return iterator to Node, or rend() otherwise.
     */
    List::const_reverse_iterator List::rfind(Node* node_) const
    {
        const_iterator found = this->find(node_);
        if (found == this->end())
        {
            return rend();
        }
        else
        {
            return const_reverse_iterator(++found);
        }
    }

    /**
     *  \brief Initialise new ListNode for Node.
     */
    ListNode* List::_new(Node* node_)
    {
        size_t index = hashOf(node_) % this->_capacity;
        ListNode* cursor = this->_data + index;
        ListNode* empty = 0;
        int fault = Utopia_MAX_FAULT;
        while (cursor->data != node_ && fault > 0)
        {
            if (cursor->data == 0 && empty == 0)
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
                return this->_new(node_);
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
     *  \brief Resize capacity of this List.
     */
    void List::_resize()
    {
#ifdef DEBUG
//         qDebug() << "Resizing List, capacity before" << this->_capacity << "=>";
#endif
        // Increase capacity
        this->_capacity = 2 * this->_capacity + 1;
#ifdef DEBUG
//         qDebug() << "  capacity after" << this->_capacity;
#endif
        // Get a handle to the old data
        ListNode* oldData = this->_data;
        // Get a handle to the old head
        ListNode* cursor = this->_head;
        // Create List data
        this->_data = new ListNode[this->_capacity + Utopia_MAX_FAULT];
        // Reset pointers
        this->_head = 0;
        this->_tail = 0;
        this->_size = 0;
        // Transfer data to new memory
        while (cursor)
        {
            this->push_back(cursor->data);
            cursor = cursor->next;
        }
        // Delete old memory
        delete [] oldData;
    }

    /**
     *  \brief Resolve Node to ListNode.
     */
    ListNode* List::_resolve(Node* node_) const
    {
        size_t index = hashOf(node_) % this->_capacity;
        ListNode* cursor = this->_data + index;
        int fault = Utopia_MAX_FAULT;
        while (cursor->data != node_ && fault > 0)
        {
            ++cursor;
            --fault;
        }
        return fault > 0 ? cursor : 0;
    }

    /**
     *  \brief Constructor for a List iterator.
     *  \param list_ List to iterate over.
     *  \param cursor_ Node to begin with.
     */
    List::iterator::iterator(ListNode* last_, ListNode* cursor_)
        : _last(last_), _cursor(cursor_)
    {}

    /**
     *  \brief Copy constructor for a List iterator.
     *  \param rhs_ iterator to copy.
     */
    List::iterator::iterator(const iterator& rhs_)
        : _last(rhs_._last), _cursor(rhs_._cursor)
    {}

    /**
     *  \brief Assignment operator for an iterator.
     *  \param rhs_ source of assignment.
     *  \return self.
     */
    List::iterator& List::iterator::operator = (const iterator& rhs_)
    {
        this->_last = rhs_._last;
        this->_cursor = rhs_._cursor;
        return *this;
    }

    /**
     *  \brief Equality operator for an iterator.
     *  \param rhs_ iterator for which equality is being tested.
     *  \return equality.
     */
    bool List::iterator::operator == (const iterator& rhs_) const
    {
        return this->_cursor == rhs_._cursor;
    }

    /**
     *  \brief Inequality operator for an iterator.
     *  \param rhs_ iterator for which inequality is being tested.
     *  \return inequality.
     */
    bool List::iterator::operator != (const iterator& rhs_) const
    {
        return !(*this == rhs_);
    }

    /**
     *  \brief Dereference operator for an iterator.
     *  \return Dereferenced Node.
     */
    List::iterator::reference List::iterator::operator * () const
    {
        return this->_cursor->data;
    }

    /**
     *  \brief Pre-increment operator for an iterator.
     *  \return self.
     */
    List::iterator& List::iterator::operator ++ ()
    {
        this->_cursor = this->_cursor->next;
        return *this;
    }

    /**
     *  \brief Post-increment operator for an iterator.
     *  \return new iterator.
     */
    List::iterator List::iterator::operator ++ (int)
    {
        iterator tmp(*this);
        ++*this;
        return tmp;
    }

    /**
     *  \brief Pre-decrement operator for an iterator.
     *  \return self.
     */
    List::iterator& List::iterator::operator -- ()
    {
        if (this->_cursor)
        {
            this->_cursor = this->_cursor->prev;
        }
        else
        {
            this->_cursor = this->_last;
        }
        return *this;
    }

    /**
     *  \brief Post-decrement operator for an iterator.
     *  \return new iterator.
     */
    List::iterator List::iterator::operator -- (int)
    {
        iterator tmp(*this);
        --*this;
        return tmp;
    }

    /**
     *  \brief Constructor for a List const_iterator.
     *  \param list_ List to iterate over.
     *  \param cursor_ Node to begin with.
     */
    List::const_iterator::const_iterator(ListNode* last_, ListNode* cursor_)
        : iterator(last_, cursor_)
    {}

    /**
     *  \brief Assignment operator for an const_iterator.
     *  \param rhs_ source of assignment.
     */
    List::const_iterator& List::const_iterator::operator = (const iterator& rhs_)
    {
        this->iterator::operator = (rhs_);
        return *this;
    }

    /**
     *  \brief Dereference operator for an const_iterator.
     *  \return Dereferenced Node.
     */
    List::const_iterator::reference List::const_iterator::operator * () const
    {
        return (List::const_iterator::reference) this->iterator::operator * ();
    }

    /**
     *  \brief Pre-increment operator for an const_iterator.
     *  \return self.
     */
    List::const_iterator& List::const_iterator::operator ++ ()
    {
        return (List::const_iterator&) this->iterator::operator ++ ();
    }

    /**
     *  \brief Post-increment operator for an const_iterator.
     *  \return new const_iterator.
     */
    List::const_iterator List::const_iterator::operator ++ (int)
    {
        return this->iterator::operator ++ (0);
    }

    /**
     *  \brief Pre-decrement operator for an const_iterator.
     *  \return self.
     */
    List::const_iterator& List::const_iterator::operator -- ()
    {
        return (List::const_iterator&) this->iterator::operator -- ();
    }

    /**
     *  \brief Post-decrement operator for an const_iterator.
     *  \return new const_iterator.
     */
    List::const_iterator List::const_iterator::operator -- (int)
    {
        return this->iterator::operator -- (0);
    }

} // namespace Utopia
