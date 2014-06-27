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

int main(int argc, char **argv)
{

    if (argc!=2) {
        cerr << "usage: " << argv[0] << " example_norm.txt" << endl;
        exit (1);
    }

    ifstream fs8(argv[1]);
    if (!fs8.is_open()) {
        cout << "Could not open " << argv[1] << endl;
        return 0;
    }

    string line;
    while (getline(fs8, line)) {

        string norm;
        normalize_utf8(line.begin(), line.end(), back_inserter(norm), utf8::NFKC);

        cout << line << "\t";
        for(string::const_iterator i=line.begin(); i!=line.end(); ++i) {
            cout << setw(2) << setfill('0') << hex << (*i & 0xff);
        }
        cout << "\t";

        cout << norm << "\t";
        for(string::const_iterator i=norm.begin(); i!=norm.end(); ++i) {
            cout << setw(2) << setfill('0') << hex << (*i & 0xff);
        }
        cout << endl;
    }

}
