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

#ifndef GTL_ALGORITHM_INCL_
#define GTL_ALGORITHM_INCL_

#include <functional>
#include <gtl/config.h>
#include <gtl/gl.h>

namespace gtl {

    /**
     *  \class  GammaCorrection
     *  \brief  Function object to modify the gamma of colour
     *          components in a buffer.
     */
    template<class CellType>
    class GammaCorrection : public std::unary_function<CellType, void>
    {
    public:
        GammaCorrection (float gamma_)
            :_gamma (gamma_)
            {}

        void operator() (CellType &cell_)
            {
                color3f colour;
                cell_.get (colour);
                colour.gamma (_gamma);
                cell_.set (colour);
            }

    private:
        float _gamma;
    };

    /**
     *  \class  GrayscaleLuminance
     *  \brief  Function object to modify a colour to its grey
     */
    template<class CellType>
    class GrayscaleLuminance : public std::unary_function<CellType, void>
    {
    public:

        void operator() (CellType &cell_)
            {
                color3f colour;
                cell_.get (colour);
                float luminance (colour.luminance());
                cell_.set (color3f (luminance, luminance, luminance));
            }
    };

    template<class CellType>
    class GrayscaleTone : public std::unary_function<CellType, void>
    {
    public:

        GrayscaleTone (gtl::color3f tone_)
            {
                double L;
                tone_.toLab (L, this->_tone_a, this->_tone_b);
            }

        void operator() (CellType &cell_)
            {
                gtl::color3f colour;
                cell_.get (colour);

                double L, a, b;
                colour.toLab (L, a, b);
                colour.fromLab (L, _tone_a, _tone_b);

                cell_.set (colour);
            }

    private:
        double _tone_a;
        double _tone_b;
    };

}

#endif /* GTL_ALGORITHM_INCL_ */
