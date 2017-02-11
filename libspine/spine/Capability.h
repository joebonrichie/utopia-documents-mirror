/*****************************************************************************
 *  
 *   This file is part of the libspine library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libspine library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libspine library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libspine library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef CAPABILITY_INCL_
#define CAPABILITY_INCL_

/*****************************************************************************
 *
 * Capability.h
 *
 * Copyright 2008 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace Spine {

    class Capability
    {
    public:
        virtual ~Capability() {}
    };

    typedef boost::shared_ptr< Capability > CapabilityHandle;
    typedef boost::weak_ptr< Capability > WeakCapabilityHandle;

}

#endif /* CAPABILITY_INCL_ */
