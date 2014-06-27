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

/*****************************************************************************
 *
 * norm_test.cpp
 *
 * Copyright 2012 Advanced Interfaces Group
 *
 ****************************************************************************/

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#include <utf8/unicode.h>

using namespace std;
using namespace utf8;

class compare_uri
{
public:
    bool operator () (const string & lhs, const string & rhs)
    {
        string::const_iterator l_begin(lhs.begin());
        string::const_iterator r_begin(rhs.begin());
        string::const_iterator l_end(lhs.end());
        string::const_iterator r_end(rhs.end());

        string::const_iterator l(l_begin);
        string::const_iterator r(r_begin);

        string lstr, rstr;
        while (l!=l_end && r!=r_end) {
            append(next(l, l_end), back_inserter(lstr));
            append(next(r, r_end), back_inserter(rstr));
        }

        cout << "L: " << lstr << endl;
        cout << "R: " << rstr << endl;

        while(l!=l_begin && r!=r_begin) {
            utf8::uint32_t l32(prior(l, l_begin));
            utf8::uint32_t r32(prior(r, r_begin));

            lstr.clear();
            rstr.clear();
            append(l32, back_inserter(lstr));
            append(r32, back_inserter(rstr));

            cout << lstr << " (" << l32 << ") ? ";
            cout << rstr << " (" << r32 << ")" <<endl;

            if (l32 < r32)
                return true;
            if (l32 > r32)
                return false;
        }

        return utf8::distance(l_begin, l_end) > utf8::distance(r_begin, r_end);
    }
};

int main(int argc, char **argv)
{

    if (argc!=2) {
        cerr << "usage: " << argv[0] << " example_compare.txt" << endl;
        exit (1);
    }

    ifstream fs8(argv[1]);
    if (!fs8.is_open()) {
        cout << "Could not open " << argv[1] << endl;
        return 0;
    }

    string line1, line2;
    if (getline(fs8, line1) && getline(fs8, line2)) {
        cout << "1: " << line1 << endl;
        cout << "2: " << line2 << endl;

        compare_uri c;

        cout << "Result: " << (c(line1, line2) ? "ORDERED" : "NOT ORDERED");
    }

}
