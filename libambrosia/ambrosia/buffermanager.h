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

#ifndef AMBROSIA_BUFFERMANAGER_H
#define AMBROSIA_BUFFERMANAGER_H

#include <ambrosia/config.h>
#include <utopia2/utopia2.h>

#include <string>
#include <list>
#include <ambrosia/buffer.h>
using namespace std;

namespace AMBROSIA {

    //
    // BufferManager class
    //

    class LIBAMBROSIA_API BufferManager {

    public:
        // Constructor
        BufferManager(string = "position:normal:rgba", size_t defaultBufferSize = 32 * 1024 * 1024);
        // Destructor
        ~BufferManager();

        // General methods
        size_t getVertexLength();
        Buffer * getBuffer(size_t);

        // OpenGL methods
        void load();
        void render(unsigned int);
        void unload();

        // Buffer methods
        void add(Buffer * buffer);
        void erase(Buffer * buffer);

        list< Buffer * > buffers;

    private:
        // BufferManager
        string format;
        size_t defaultBufferSize;
        size_t vertexLength;

    }; // class BufferManager

} // namespace AMBROSIA

#endif // AMBROSIA_BUFFERMANAGER_H
