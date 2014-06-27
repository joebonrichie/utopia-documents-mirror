/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#ifndef AMBROSIA_COLOUR_H
#define AMBROSIA_COLOUR_H

#include <ambrosia/config.h>
#include <utopia2/utopia2.h>
#include <map>
#include <string>
using namespace std;

namespace AMBROSIA {

    //
    // class Colour
    //

    class LIBAMBROSIA_API Colour {

    public:
        // Contsructor
        Colour(unsigned char, unsigned char, unsigned char);

        // colour values
        unsigned char r;
        unsigned char g;
        unsigned char b;
        void set(unsigned char, unsigned char, unsigned char);
        void get(unsigned char &, unsigned char &, unsigned char &);
        void getf(float &, float &, float &);

        // Static Methods
        static void populate(string);
        static Colour * getColour(string);
        static Colour * getColour(unsigned char, unsigned char, unsigned char);
        static Colour * getColour(string, unsigned char, unsigned char, unsigned char);

    private:
        // Static list of all colours...
        static map< string, Colour * > all;

    }; // class Colour

} // namespace AMBROSIA

#endif // AMBROSIA_COLOUR_H
