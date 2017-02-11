/*****************************************************************************
 *  
 *   This file is part of the libcrackle library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libcrackle library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libcrackle library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libcrackle library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef SIMPLECOLLECTION_INCL_
#define SIMPLECOLLECTION_INCL_

/*****************************************************************************
 *
 * SimpleCollection.h
 *
 * Simple STL-style proxying class for singly linked list
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <iterator>
#include <iostream>
#include <vector>

namespace Crackle
{

/*
  template<class T_PROXY>
  class SimpleCollection;

  template<class T_PROXY>
  class SimpleCollection_const_iterator
  : public std::iterator<std::forward_iterator_tag, const T_PROXY>
  {

  public:

  SimpleCollection_const_iterator()
  : _proxy(0)
  {
  }

  SimpleCollection_const_iterator(const SimpleCollection_const_iterator& rhs_)
  : _proxy(rhs_._proxy)
  {
  }

  virtual ~SimpleCollection_const_iterator()
  {
  }

  SimpleCollection_const_iterator& operator=(const SimpleCollection_const_iterator &rhs_)
  {
  if (&rhs_!=this) {
  _proxy=rhs_._proxy;
  }
  return *this;
  }

  bool operator==(const SimpleCollection_const_iterator &rhs_) const
  {
  return _proxy==rhs_._proxy;
  }

  bool operator!=(const SimpleCollection_const_iterator &rhs_) const
  {
  return !(*this==rhs_);
  }

  const T_PROXY& operator*() const
  {
  return _proxy;
  }

  const T_PROXY* operator->() const
  {
  return (&*(*this));
  }

  SimpleCollection_const_iterator& operator++()
  {
  _proxy.advance();
  return *this;
  }

  SimpleCollection_const_iterator operator++(int)
  {
  SimpleCollection_const_iterator tmp(*this);
  ++(*this);
  return (tmp);
  }

  private:

  friend class SimpleCollection<T_PROXY>;

  SimpleCollection_const_iterator(typename T_PROXY::wrapped_class *ptr_)
  : _proxy(ptr_)
  {
  }

  T_PROXY _proxy;
  };

  template<class T_PROXY>
  class SimpleCollection
  {

  public:

  typedef const T_PROXY  value_type;
  typedef const T_PROXY& reference;
  typedef const T_PROXY* pointer;
  typedef SimpleCollection_const_iterator<T_PROXY> const_iterator;

  virtual ~SimpleCollection()
  {}

  const_iterator begin() const
  {
  return const_iterator(_first);
  }

  const_iterator end() const
  {
  return const_iterator(0);
  }

  size_t size() const
  {
  size_t sz(0);
  for(const_iterator i(this->begin()); i!=this->end(); ++i) {
  ++sz;
  }
  return sz;
  }

  protected:

  SimpleCollection(typename T_PROXY::wrapped_class *first_)
  :_first (first_)
  { }

  typename T_PROXY::wrapped_class * _first;

  };
*/


    template< class T_PROXY >
    class SimpleCollection : public std::vector< T_PROXY >
    {
        typedef std::vector< T_PROXY > _Base;

    public:
        SimpleCollection(typename T_PROXY::wrapped_class * ptr)
            : _Base()
            {
                while (ptr)
                {
                    _Base::push_back(T_PROXY(ptr));
                    ptr = ptr->getNext();
                }
            }

        friend class PDFPage;
    };

}

#endif /* SIMPLECOLLECTION_INCL_ */
