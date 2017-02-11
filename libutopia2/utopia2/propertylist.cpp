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

#include <utopia2/propertylist.h>
#include <utopia2/node.h>

#ifdef DEBUG
#   include <iostream>
#endif

namespace Utopia
{

    /** \brief Constructs _PropertyList object. */
    _PropertyList::_PropertyList(Node* node_, const Property& property_)
        : _node(node_), _property(property_)
    {
        _list = node_->relations._getDirectAccessList(property_);
    }

    /** \brief Return iterator to the first Element in this Property List. */
    _PropertyList::iterator _PropertyList::begin()
    {
        if (_list)
        {
            return iterator(_node, _property, _list, _list->begin(), false);
        }
        else
        {
            return iterator();
        }
    }

    /** \brief Return const_iterator to the first Element in this Property List. */
    _PropertyList::const_iterator _PropertyList::begin() const
    {
        if (_list)
        {
            return const_iterator(_node, _property, _list, _list->begin(), false);
        }
        else
        {
            return const_iterator();
        }
    }

    /** \brief Return iterator to the end of this Property List. */
    _PropertyList::iterator _PropertyList::end()
    {
        if (_list)
        {
            return iterator(_node, _property, _list, _list->end(), false);
        }
        else
        {
            return iterator();
        }
    }

    /** \brief Return const_iterator to the end of this Property List. */
    _PropertyList::const_iterator _PropertyList::end() const
    {
        if (_list)
        {
            return const_iterator(_node, _property, _list, _list->end(), false);
        }
        else
        {
            return const_iterator();
        }
    }

    /** \brief Find a Node in this Property List and return its iterator. */
    _PropertyList::iterator _PropertyList::find(Node* node_)
    {
        if (_list)
        {
            List::iterator list_iter = _list->find(node_);
            if (list_iter != _list->end())
            {
                return iterator(_node, _property, _list, list_iter, false);
            }
        }

        return end();
    }

    /** \brief Find a Node in this Property List and return its const_iterator. */
    _PropertyList::const_iterator _PropertyList::find(Node* node_) const
    {
        if (_list)
        {
            List::iterator list_iter = _list->find(node_);
            if (list_iter != _list->end())
            {
                return const_iterator(_node, _property, _list, list_iter, false);
            }
        }

        return end();
    }

    /** \brief Return reverse_iterator to the first Element in this Property List. */
    _PropertyList::reverse_iterator _PropertyList::rbegin()
    {
        return _PropertyList::reverse_iterator(end());
    }

    /** \brief Return const_reverse_iterator to the first Element in this Property List. */
    _PropertyList::const_reverse_iterator _PropertyList::rbegin() const
    {
        return _PropertyList::const_reverse_iterator(end());
    }

    /** \brief Return reverse_iterator to the end of this Property List. */
    _PropertyList::reverse_iterator _PropertyList::rend()
    {
        return _PropertyList::reverse_iterator(begin());
    }

    /** \brief Return const_reverse_iterator to the end of this Property List. */
    _PropertyList::const_reverse_iterator _PropertyList::rend() const
    {
        return _PropertyList::const_reverse_iterator(begin());
    }

    /**
     *  \brief Append Node to the relation List.
     *
     *  Cannot push a Node onto a relation List more than once.
     */
    void _PropertyList::append(Node* node_)
    {
        // FIXME Include reflexivity

        if (!_list)
        {
            _list = _node->relations._getDirectAccessList(_property, true);
        }

        // Append object to subject's relation List
        _list->push_back(node_);

        // For reverse Property, add subject to object's relation List
        node_->relations._getDirectAccessList(~_property, true)->push_back(_node);
    }

    Node* _PropertyList::at(size_t index_)
    {
        iterator iter = this->begin();
        iterator end = this->end();
        for (; iter != end && index_ > 0; ++iter, --index_) {}
        return iter != end ? *iter : 0;
    }

    /** \brief Return the last Node in the relation List. */
    Node* _PropertyList::back()
    {
        return _list && !_list->empty() ? _list->back() : 0;
    }

    /** \brief Return the last Node in the relation List. */
    Node* const _PropertyList::back() const
    {
        return _list && !_list->empty() ? _list->back() : 0;
    }

    /** \brief Clear the relation List. */
    void _PropertyList::clear()
    {
        if (_list)
        {
            List::iterator iter = _list->begin();
            List::iterator end = _list->end();
            while (iter != end)
            {
                List* os_list = (*iter)->relations._getDirectAccessList(~_property);
                os_list->erase(os_list->find(_node));

                if (os_list->empty())
                {
                    (*iter)->relations._relations.erase(~_property);
                    delete os_list;
                }

                ++iter;
            }
            _list->clear();
            _node->relations._relations.erase(_property);
            delete _list;
            _list = 0;
        }
    }

    /** \brief Returns whether this Property List is empty. */
    bool _PropertyList::empty() const
    {
        return !_list || _list->empty();
    }

    /** \brief Returns whether this Property List includes a particular Node. */
    bool _PropertyList::exists(Node* node_) const
    {
        return _list && _list->exists(node_);
    }

    /** \brief Return the first Node in the relation List. */
    Node* _PropertyList::front()
    {
        return _list && !_list->empty() ? _list->front() : 0;
    }

    /** \brief Return the first Node in the relation List. */
    Node* const _PropertyList::front() const
    {
        return _list && !_list->empty() ? _list->front() : 0;
    }

    /** \brief Remove a Node from the relation List. */
    Node* _PropertyList::remove(Node* node_)
    {
        List* os_list = node_->relations._getDirectAccessList(~_property);

        if (_list && os_list)
        {
            // Remove object from subject's relation List
            _list->erase(_list->find(node_));
            // Remove subject from object's relation List
            os_list->erase(os_list->find(_node));

            // Clean up if need be
            if (_list->empty())
            {
                _node->relations._relations.erase(_property);
                delete _list;
                _list = 0;
            }
            if (os_list->empty())
            {
                node_->relations._relations.erase(~_property);
                delete os_list;
            }

            return node_;
        }
        else
        {
            return 0;
        }
    }

    /** \brief Returns the size of this Property List (i.e. the sum of all elements). */
    size_t _PropertyList::size() const
    {
        return _list ? _list->size() : 0;
    }



    //
    // class _PropertyList::iterator
    //

    /**
     *  \brief Sets up an iterator across these direct access lists.
     *
     *  Creates an iterator that points to the first Node of the first List.
     */
    _PropertyList::iterator::iterator(Node* node_, const Property& property_, List* list_, List::iterator listIterator_, bool recursive_)
        : _node(node_), _property(property_), _list(list_), _recursive(recursive_), _recursiveIterator(0)
    {
        _listIterator = new List::iterator(listIterator_);
    }

    /**
     *  \brief Sets up an iterator across NO direct access List.
     *
     *  Creates an iterator that points to the first Node of the first List.
     */
    _PropertyList::iterator::iterator()
        : _node(0), _property(0), _list(0), _listIterator(0), _recursive(false), _recursiveIterator(0)
    {}

    _PropertyList::iterator::~iterator()
    {
        if (_recursiveIterator)
        {
            delete _recursiveIterator;
        }
        if (_listIterator)
        {
            delete _listIterator;
        }
    }

    /**
     *  \brief Copy constructor for an iterator.
     */
    _PropertyList::iterator::iterator(const _PropertyList::iterator& rhs_)
        : _listIterator(0), _recursiveIterator(0)
    {
        *this = rhs_;
    }

    /**
     *  \brief Assignment operator for an iterator.
     */
    _PropertyList::iterator& _PropertyList::iterator::operator = (const _PropertyList::iterator& rhs_)
    {
        _node = rhs_._node;
        _list = rhs_._list;
        if (_listIterator)
        {
            delete _listIterator;
            _listIterator = 0;
        }
        if (rhs_._listIterator)
        {
            _listIterator = new List::iterator(*rhs_._listIterator);
        }
        _recursive = rhs_._recursive;
        if (_recursiveIterator)
        {
            delete _recursiveIterator;
            _recursiveIterator = 0;
        }
        if (rhs_._recursiveIterator)
        {
            _recursiveIterator = new iterator(*rhs_._recursiveIterator);
        }
        return *this;
    }

    /**
     *  \brief Equality operator for an iterator.
     */
    bool _PropertyList::iterator::operator == (const _PropertyList::iterator& rhs_) const
    {
        bool equal = true;
        if (_list)
        {
            equal = equal && (_list == rhs_._list);
            equal = equal && (*_listIterator == *rhs_._listIterator);
            if (_recursiveIterator && rhs_._recursiveIterator)
            {
                equal = equal && (*_recursiveIterator == *rhs_._recursiveIterator);
            }
        }
        else
        {
            equal = equal && (_list == rhs_._list);
        }
        return equal;
    }

    /**
     *  \brief Inequality operator for an iterator.
     */
    bool _PropertyList::iterator::operator != (const _PropertyList::iterator& rhs_) const
    {
        return !(*this == rhs_);
    }

    /**
     *  \brief Dereference operator for an iterator.
     */
    _PropertyList::iterator::reference _PropertyList::iterator::operator * () const
    {
        return _recursiveIterator == 0 ? **_listIterator : **_recursiveIterator;
    }

    /**
     *  \brief Pre-increment operator for an iterator.
     */
    _PropertyList::iterator& _PropertyList::iterator::operator ++ ()
    {
        // If already at the end, then do nothing, else...
        if (_list)
        {
            // If recursive...
            if (_recursive)
            {
                // ...then recurse if not already
                if (_recursiveIterator == 0)
                {
                    _recursiveIterator = new iterator(_node->relations(_property).begin());
                }
                // Else increment current recursive iterator
                else
                {
                    ++*_recursiveIterator;
                }

                // If current recursive iterator is at END...
                if (*_recursiveIterator == iterator(_node->relations(_property).end()))
                {
                    // ...then delete iterator ready to move on
                    delete _recursiveIterator;
                    _recursiveIterator = 0;
                }
                // Else return
                else
                {
                    return *this;
                }
            }

            // Increment Node in List
            ++*_listIterator;
        }

        return *this;
    }

    /**
     *  \brief Post-increment operator for an iterator.
     */
    _PropertyList::iterator _PropertyList::iterator::operator ++ (int)
    {
        _PropertyList::iterator tmp(*this);
        ++*this;
        return tmp;
    }

    /**
     *  \brief Pre-decrement operator for an iterator.
     */
    _PropertyList::iterator& _PropertyList::iterator::operator -- ()
    {
        // FIXME (recursive)
        if (_list)
        {
            --*_listIterator;
        }

        return *this;
    }

    /**
     *  \brief Post-decrement operator for an iterator.
     */
    _PropertyList::iterator _PropertyList::iterator::operator -- (int)
    {
        _PropertyList::iterator tmp(*this);
        --*this;
        return tmp;
    }



    //
    // class _PropertyList::const_iterator
    //

    /**
     *  \brief Sets up an const_iterator across these direct access lists.
     *
     *  Creates an const_iterator that points to the first Node of the first List.
     */
    _PropertyList::const_iterator::const_iterator()
        : _PropertyList::iterator()
    {}

    /**
     *  \brief Sets up an const_iterator across these direct access lists.
     *
     *  Creates an const_iterator that points to the given Node of the given List.
     */
    _PropertyList::const_iterator::const_iterator(Node* node_, const Property& property_, List* list_, List::iterator listIterator_, bool recursive_)
        : _PropertyList::iterator(node_, property_, list_, listIterator_, recursive_)
    {}

    /**
     *  \brief Assignment operator for a const_iterator.
     */
    _PropertyList::const_iterator& _PropertyList::const_iterator::operator = (const _PropertyList::iterator& rhs_)
    {
        this->iterator::operator=(rhs_);
        return *this;
    }

    /**
     *  \brief Dereference operator for a const_iterator.
     */
    _PropertyList::const_iterator::reference _PropertyList::const_iterator::operator * () const
    {
        return (_PropertyList::const_iterator::reference) this->iterator::operator*();
    }

} // namespace Utopia
