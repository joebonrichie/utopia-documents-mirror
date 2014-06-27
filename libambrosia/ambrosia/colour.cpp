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

#include <ambrosia/colour.h>
#include <utopia2/global.h>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

namespace AMBROSIA {

    //
    // class Colour
    //

    Colour::Colour(unsigned char red, unsigned char green, unsigned char blue) {
        this->r = red;
        this->g = green;
        this->b = blue;
    }

    void Colour::set(unsigned char red, unsigned char green, unsigned char blue) {
        this->r = red;
        this->g = green;
        this->b = blue;
    }

    void Colour::get(unsigned char & red, unsigned char & green, unsigned char & blue) {
        red = this->r;
        green = this->g;
        blue = this->b;
    }

    void Colour::getf(float & red, float & green, float & blue) {
        red = this->r / 256.0;
        green = this->g / 256.0;
        blue = this->b / 256.0;
    }

    void Colour::populate(string filename) {
        // Read in from file
        if (filename.size() > 0 && filename.at(0) != '/') {
            filename = (Utopia::resource_path() + "/ambrosia/colourmaps/" + filename.c_str()).toUtf8().constData();
        }
        ifstream colourmap(filename.c_str());
        if (colourmap.is_open()) {
            while (!colourmap.eof()) {
                string name;
                int r, g, b;
                colourmap >> name >> r >> g >> b;
                if (all.find(name) == all.end())
                    all[name] = new Colour(r, g, b);
                else
                    all[name]->set(r, g, b);
            }
        } else {
            qDebug() << QString::fromStdString(string("Unable to open colour map \"") + filename + string("\""));
        }
    }

    Colour * Colour::getColour(string name) {
        if (all.find(name) != all.end())
            return all[name];
        else
            return all["?"];
    }
    Colour * Colour::getColour(unsigned char r, unsigned char g, unsigned char b) {
        stringstream namestr;
        namestr << "user." << (int) r << "." << (int) g << "." << (int) b;
        string name = namestr.str();
        if (all.find(name) != all.end()) {
            return all[name];
        } else {
            all[name] = new Colour(r, g, b);
            return all[name];
        }
    }
    Colour * Colour::getColour(string name, unsigned char r, unsigned char g, unsigned char b) {
        if (all.find(name) != all.end()) {
            all[name]->set(r, g, b);
        } else {
            all[name] = new Colour(r, g, b);
        }
        return all[name];
    }

    // Static list of all colours...
    map< string, Colour * > Colour::all;

} // namespace AMBROSIA
