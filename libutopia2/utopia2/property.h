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

#ifndef Utopia_PROPERTY_H
#define Utopia_PROPERTY_H

#include <utopia2/config.h>
#include <utopia2/enums.h>

namespace Utopia
{

    // Forward declarations
    class Node;

    /**
     *  \class Property;
     */
    class LIBUTOPIA_API Property
    {
    public:
        /** \name Construction and destruction. */
        //@{

        Property();
        Property(Node* rhs_, PropertyDirection dir_ = Forward);
        Property(const Property& rhs_);

        //@}
        /** \name Pointer funcionality. */
        //@{

        operator bool ();
        operator Node* ();
        operator const Node* () const;
        bool operator ! () const;
        bool operator < (const Property& rhs_) const;
        Node& operator * ();
        const Node& operator * () const;
        Node* operator -> ();
        const Node* operator -> () const;
        Property& operator = (const Property& rhs_);
        Property& operator = (Node* rhs_);
        bool operator == (const Property& rhs_) const;
        bool operator != (const Property& rhs_) const;

        //@}
        /** \name Property methods. */
        //@{

        Node* data();
        const Node* data() const;
        Property operator ~ () const;
        PropertyDirection dir() const;
        unsigned long hash() const;

        //@}

    private:
        // Property
        unsigned long _property;

    }; /* class Property */

    // Make reverse Property
    LIBUTOPIA_EXPORT Property forwardProperty(Node* node_);
    // Make reverse Property
    LIBUTOPIA_EXPORT Property reverseProperty(Node* node_);
    // Make reflexive Property
    LIBUTOPIA_EXPORT Property reflexiveProperty(Node* node_);

} /* namespace Utopia */

#endif /* Utopia_PROPERTY_H */
