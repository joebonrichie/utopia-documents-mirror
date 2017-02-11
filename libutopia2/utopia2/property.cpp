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

#include <utopia2/property.h>

#define PROP2NODE(p) ((Node*) ((unsigned long long) p & ((unsigned long long) -1 << 3)))
#define PROP2DIR(p) ((PropertyDirection) (p & 0x7))
#define MAKEPROP(n, d) ((unsigned long long) n | (unsigned long long) d)

namespace Utopia
{

    // Forwards
    class Node;

    //
    // class Property
    //

    /** \brief Default constructor (NULL). */
    Property::Property()
        : _property(MAKEPROP(0, Forward))
    {}

    /** \brief Copy constructor. */
    Property::Property(Node* rhs_, PropertyDirection dir_)
        : _property(MAKEPROP(rhs_, dir_))
    {}

    /** \brief Copy constructor. */
    Property::Property(const Property& rhs_)
        : _property(rhs_._property)
    {}

    /** \brief Validity check. */
    Property::operator bool ()
    {
        return PROP2NODE(_property) != 0;
    }

    /** \brief Implicit conversion. */
    Property::operator Node* ()
    {
        return PROP2NODE(_property);
    }

    /** \brief Implicit const conversion. */
    Property::operator const Node* () const
    {
        return PROP2NODE(_property);
    }

    /** \brief Not null? */
    bool Property::operator ! () const
    {
        return !_property;
    }

    /** \brief Less than? */
    bool Property::operator < (const Property& rhs_) const
    {
        return (PROP2NODE(_property) < PROP2NODE(rhs_._property)) ||
            (PROP2NODE(_property) == PROP2NODE(rhs_._property) &&
             PROP2DIR(_property) < PROP2DIR(rhs_._property));
    }

    /** \brief Dereference. */
    Node& Property::operator * ()
    {
        return *PROP2NODE(_property);
    }

    /** \brief Const dereference. */
    const Node& Property::operator * () const
    {
        return *PROP2NODE(_property);
    }

    /** \brief Dereference. */
    Node* Property::operator -> ()
    {
        return PROP2NODE(_property);
    }

    /** \brief Const dereference. */
    const Node* Property::operator -> () const
    {
        return PROP2NODE(_property);
    }

    /** \brief Assignment operator. */
    Property& Property::operator = (const Property& rhs_)
    {
        _property = rhs_._property;
        return *this;
    }

    /** \brief Assignment operator. */
    Property& Property::operator = (Node* rhs_)
    {
        _property = MAKEPROP(rhs_, Forward);
        return *this;
    }

    /** \brief Equality operator. */
    bool Property::operator == (const Property& rhs_) const
    {
        return (_property == rhs_._property);
    }

    /** \brief Equality operator. */
    bool Property::operator != (const Property& rhs_) const
    {
        return !(*this == rhs_);
    }

    /** \brief Access data. */
    Node* Property::data()
    {
        return PROP2NODE(_property);
    }

    /** \brief Access const data. */
    const Node* Property::data() const
    {
        return PROP2NODE(_property);
    }

    /** \brief Reverse Property. */
    Property Property::operator ~ () const
    {
        switch (PROP2DIR(_property))
        {
        case Forward:
            return Property(PROP2NODE(_property), Backward);
            break;
        case Backward:
            return Property(PROP2NODE(_property), Forward);
            break;
        default:
            return *this;
            break;
        }
    }

    /** \brief Query direction. */
    PropertyDirection Property::dir() const
    {
        return PROP2DIR(_property);
    }

    /** \brief Hash this Property. */
    unsigned long Property::hash() const
    {
        return _property;
    }


    /** Make reverse Property */
    Property forwardProperty(Node* node_)
    {
        return Property(node_);
    }

    /** Make reverse Property */
    Property reverseProperty(Node* node_)
    {
        return Property(node_, Backward);
    }

    /** Make reflexive Property */
    Property reflexiveProperty(Node* node_)
    {
        return Property(node_, Reflexive);
    }

} /* namespace Utopia */
