/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#ifndef AMBROSIA_BUFFER_H
#define AMBROSIA_BUFFER_H

#include <ambrosia/config.h>
#include <utopia2/utopia2.h>

#include <string>

using namespace std;

namespace AMBROSIA {

    //
    // Buffer class
    //

    class LIBAMBROSIA_API Buffer {

    public:
        // typedef
        typedef enum {
            POSITION = 1,
            NORMAL = 2,
            COLOUR = 4,
            TEXCOORD = 8,
            ALL = 15
        } ElementType;

        // Constructor
        Buffer(string, unsigned int);
        // Destructor
        ~Buffer();

        // General methods
        void invalidate();
        void validate();
        bool isValid();
        bool isLoaded();
        unsigned int size();
        unsigned int freeSpace();
        unsigned int usedSpace();
        unsigned int usedVertices();
        unsigned int freeVertices();
        unsigned int getVertexLength();
        static unsigned int getVertexLengthFromFormat(string);

        // Data insertion methods
        void setPosition(float, float, float = 0.0, float = 0.0);
        void setNormal(float, float, float);
        void setTexCoord(float, float = 0.0, float = 0.0, float = 0.0);
        void setColourf(float, float, float, float = 1.0);
        void setColourb(unsigned char, unsigned char, unsigned char, unsigned char = 255);
        void next();
        void to(unsigned int);
        bool EOB();

        // OpenGL methods
        void load();
        void load(unsigned int, unsigned int);
        bool enable(unsigned int = ALL);
        void render(unsigned int, int = 0, int = -1);
        bool disable();
        void unload();

    private:
        // Buffer
        string format;
        unsigned int capacity;
        unsigned char * buffer;
        unsigned int length;
        // Cursor
        unsigned int cursor;
        // Status
        bool valid;
        unsigned int loadedTo;

        // Usage values
        unsigned int vertexLength;
        int positionOffset;
        unsigned int positionSize;
        int normalOffset;
        int texcoordOffset;
        unsigned int texcoordSize;
        int rgbOffset;
        int rgbaOffset;

        // VBOs
        unsigned int vbo_handle;

    }; // class Buffer

} // namespace AMBROSIA

#endif // AMBROSIA_BUFFER_H
