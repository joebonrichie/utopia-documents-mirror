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

#ifndef GTL_ORIENTATION_INCL_
#define GTL_ORIENTATION_INCL_

#include <gtl/config.h>

namespace gtl
{

    /**
     *  \class orientation
     *  \brief Representation of an orientation.
     *
     *  This class represents an orientation within 2D or 3D space.
     */
    template< typename _ComponentType, size_t _size >
    class orientation
    {};

    typedef orientation< float, 2 > orientation_2f;
    typedef orientation< float, 3 > orientation_3f;
    typedef orientation< double, 2 > orientation_2d;
    typedef orientation< double, 3 > orientation_3d;

} /* namespace gtl */

// Include partial specialisations
#include <gtl/bits/orientation2.h>
#include <gtl/bits/orientation3.h>

#endif /* GTL_ORIENTATION_INCL_ */
