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

#include <spine/Area.h>
#include <boost/foreach.hpp>
#include <vector>

#include <iostream>

namespace Spine {

    AreaList compile(const AreaList & areas)
    {
        std::vector< Area > input(areas.begin(), areas.end());
        AreaList output;
        size_t start = 0;
        while (start + 1 < input.size())
        {
            size_t index = start + 1;
            while (index < input.size() && input[start].page >= 0)
            {
                for(index = start + 1; index < input.size(); ++index)
                {
                    // Check for intersection between start and index
                    if (input[start].page == input[index].page &&
                        input[start].boundingBox.x1 <= input[index].boundingBox.x2 &&
                        input[index].boundingBox.x1 <= input[start].boundingBox.x2 &&
                        input[start].boundingBox.y1 <= input[index].boundingBox.y2 &&
                        input[index].boundingBox.y1 <= input[start].boundingBox.y2)
                    {
                        input[start].boundingBox |= input[index].boundingBox;
                        input[index] = Area();
                        break;
                    }
                }
            }
            ++start;
        }

        BOOST_FOREACH(Area & area, input)
        {
            if (area.page >= 0) { output.push_back(area); }
        }

        return output;
    }

}
