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

#include <ambrosia/buffermanager.h>
#include "ambrosia/utils.h"
#include <utopia2/utopia2.h>
#include <iostream>
using namespace std;

namespace AMBROSIA {

    //
    // BufferManager class
    //

    // Constructor
    BufferManager::BufferManager(string format, size_t defaultBufferSize)
        : format(format), defaultBufferSize(defaultBufferSize)
    {
        OpenGLSetup();

        vertexLength = 0;
        size_t delimIndex = 0;
        size_t elementIndex = 0;
        while (delimIndex != string::npos) {
            //std::cerr << elementIndex << " " << delimIndex << " " << string::npos << " " << format.size() << std::endl;
            delimIndex = format.find(':', elementIndex);
            string element = (delimIndex == string::npos) ? format.substr(elementIndex) : format.substr(elementIndex, delimIndex - elementIndex);
            if (element == "position2d")
                vertexLength += 2 * sizeof(GLfloat);
            else if (element == "position3d" || element == "position")
                vertexLength += 3 * sizeof(GLfloat);
            else if (element == "position4d")
                vertexLength += 4 * sizeof(GLfloat);
            else if (element == "normal")
                vertexLength += 3 * sizeof(GLfloat);
            else if (element == "texcoord1d")
                vertexLength += 1 * sizeof(GLfloat);
            else if (element == "texcoord2d" || element == "texcoord")
                vertexLength += 2 * sizeof(GLfloat);
            else if (element == "texcoord3d")
                vertexLength += 3 * sizeof(GLfloat);
            else if (element == "texcoord4d")
                vertexLength += 4 * sizeof(GLfloat);
            else if (element == "rgb")
                vertexLength += 3 * sizeof(GLubyte);
            else if (element == "rgba")
                vertexLength += 4 * sizeof(GLubyte);
            elementIndex = delimIndex + 1;
        }

        int MAX_ELEMENTS_VERTICES = 0;
        int ABS_MAX_ELEMENTS_VERTICES = (32 * 1024 * 1024) / vertexLength;
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, (GLint*)&MAX_ELEMENTS_VERTICES);
        if (MAX_ELEMENTS_VERTICES > ABS_MAX_ELEMENTS_VERTICES)
            MAX_ELEMENTS_VERTICES = ABS_MAX_ELEMENTS_VERTICES;
        this->defaultBufferSize = vertexLength * MAX_ELEMENTS_VERTICES;
    }

    // Destructor
    BufferManager::~BufferManager()
    {
        // Delete all buffers controlled by this manager
    }

    // General methods
    size_t BufferManager::getVertexLength()
    { return vertexLength; }
    Buffer * BufferManager::getBuffer(size_t verticesRequired)
    {
        Buffer * buffer = 0;

        // Check if defaultBufferSize is big enough
        size_t bufferLength = defaultBufferSize / vertexLength;
        if (verticesRequired > bufferLength) {
            bufferLength = verticesRequired;
            char log[200];
            sprintf(log, "Ambrosia: %.1f KB vertex buffer requested that exceeds default size of %.1f KB", bufferLength * vertexLength / (1024.0), defaultBufferSize / (1024.0));
        }

        list< Buffer * >::iterator buffer_iter = buffers.begin();
        list< Buffer * >::iterator buffer_end = buffers.end();
        for (; buffer_iter != buffer_end; ++buffer_iter) {
            if ((*buffer_iter)->freeVertices() >= verticesRequired) {
                buffer = *buffer_iter;
                break;
            }
        }
        if (buffer_iter == buffer_end) {
            buffer = new Buffer(format, bufferLength);
            buffers.push_back(buffer);
        }

        return buffer;
    }

    // OpenGL methods
    void BufferManager::load()
    {
        // Load all buffers
        list< Buffer * >::iterator buffer = buffers.begin();
        list< Buffer * >::iterator buffer_end = buffers.end();
        for (; buffer != buffer_end; ++buffer) {
            (*buffer)->load();
        }
    }
    void BufferManager::unload()
    {
        // unload all buffers
        list< Buffer * >::iterator buffer = buffers.begin();
        list< Buffer * >::iterator buffer_end = buffers.end();
        for (; buffer != buffer_end; ++buffer) {
            (*buffer)->unload();
        }
    }
    void BufferManager::render(unsigned int mode)
    {
        // render all buffers of mode
        list< Buffer * >::iterator buffer = buffers.begin();
        list< Buffer * >::iterator buffer_end = buffers.end();
        for (; buffer != buffer_end; ++buffer) {
            (*buffer)->enable();
            (*buffer)->render(mode);
            (*buffer)->disable();
        }
    }
    void BufferManager::add(Buffer * buffer)
    { buffers.push_back(buffer); }
    void BufferManager::erase(Buffer * buffer)
    { buffers.remove(buffer); }

} // namespace AMBROSIA
