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

#ifndef Utopia_FUNCTIONAL_H
#define Utopia_FUNCTIONAL_H

#include <utopia2/config.h>
#include <utopia2/node.h>

#include <QString>

namespace Utopia
{
    class LIBUTOPIA_API criterion
    {
    public:
        virtual ~criterion() {};
        virtual bool operator () (Node* node_) = 0;

    }; /* class criterion */

    class LIBUTOPIA_API AND : public criterion
    {
    public:
        AND(criterion* lhs_, criterion* rhs_);
        ~AND();
        bool operator () (Node* node_);

    private:
        criterion* _lhs;
        criterion* _rhs;
    }; /* class AND */

    class LIBUTOPIA_API OR : public criterion
    {
    public:
        OR(criterion* lhs_, criterion* rhs_);
        ~OR();
        bool operator () (Node* node_);

    private:
        criterion* _lhs;
        criterion* _rhs;
    }; /* class OR */

    class LIBUTOPIA_API HasAttribute : public criterion
    {
    public:
        HasAttribute(QString key_);
        ~HasAttribute();
        bool operator () (Node* node_);

    private:
        QString _key;
    }; /* class HasAttribute */

    template< typename value_type >
    class LIBUTOPIA_API HasAttributeValue : public criterion
    {
    public:
        HasAttributeValue(QString key_, value_type value_)
            : _key(key_), _value(value_)
        {}

        ~HasAttributeValue()
        {}

        bool operator () (Node* node_)
        {
            return node_->attributes.exists(_key) && node_->attributes.get(_key).template value< value_type >() == _value;
        }

    private:
        QString _key;
        value_type _value;
    }; /* class HasAttributeValue */

    class LIBUTOPIA_API AtPosition : public criterion
    {
    public:
        AtPosition(int index_);

        ~AtPosition();

        bool operator () (Node* node_);

    private:
        int _index;
        bool _found;
    }; /* class AtPosition */

} /* namespace Utopia */

#endif /* Utopia_FUNCTIONAL_H */
