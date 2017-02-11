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

#ifndef GTL_SCALE_INCL_
#define GTL_SCALE_INCL_

#include <gtl/config.h>

namespace gtl
{

    /**
     *  \class scale
     *  \brief Representation of a scaling.
     *
     *  This class represents a scaling within 2D or 3D space.
     */
    template< typename _ComponentType, size_t _size >
    class scale
    {};

    typedef scale< float, 2 > scale_2f;
    typedef scale< float, 3 > scale_3f;
    typedef scale< double, 2 > scale_2d;
    typedef scale< double, 3 > scale_3d;

} /* namespace gtl */

// Include partial specialisations
#include <gtl/bits/scale2.h>
#include <gtl/bits/scale3.h>

#endif /* GTL_SCALE_INCL_ */
