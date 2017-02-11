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

#include <utopia2/functional.h>

namespace Utopia
{
    AND::AND(criterion* lhs_, criterion* rhs_)
        : _lhs(lhs_), _rhs(rhs_)
    {}

    AND::~AND()
    {}

    bool AND::operator () (Node* node_)
    {
        return ((*_lhs)(node_) && (*_rhs)(node_));
    }

    OR::OR(criterion* lhs_, criterion* rhs_)
        : _lhs(lhs_), _rhs(rhs_)
    {}

    OR::~OR()
    {}

    bool OR::operator () (Node* node_)
    {
        return ((*_lhs)(node_) || (*_rhs)(node_));
    }

    HasAttribute::HasAttribute(QString key_)
        : _key(key_)
    {}

    HasAttribute::~HasAttribute()
    {}

    bool HasAttribute::operator () (Node* node_)
    {
        return node_->attributes.exists(_key);
    }

    AtPosition::AtPosition(int index_)
        : _index(index_), _found(false)
    {}

    AtPosition::~AtPosition()
    {}

    bool AtPosition::operator () (Node* node_)
    {
        if (!_found)
        {
            if (_index == 0)
            {
                return (_found = true);
            }
            else if (_index > 0)
            {
                --_index;
            }
            else
            {
                ++_index;
            }
        }

        return false;
    }

} /* namespace Utopia */
