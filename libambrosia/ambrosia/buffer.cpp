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

#include <ambrosia/buffer.h>
#include "ambrosia/utils.h"
#include <utopia2/utopia2.h>
#include <iostream>
using namespace std;

int mem = 0;

namespace AMBROSIA {

    //
    // Buffer class
    //

    // Constructor
    Buffer::Buffer(string format, unsigned int capacity)
        : format(format), capacity(capacity), length(0), cursor(0), valid(true), loadedTo(0), positionOffset(-1), positionSize(3), normalOffset(-1), texcoordOffset(-1), texcoordSize(2), rgbOffset(-1), rgbaOffset(-1), vbo_handle(0)
    {
        OpenGLSetup();

        size_t offset = 0;
        size_t delimIndex = 0;
        size_t elementIndex = 0;
        while (delimIndex != string::npos) {
            delimIndex = format.find(':', elementIndex);
            string element = (delimIndex == string::npos) ? format.substr(elementIndex) : format.substr(elementIndex, delimIndex - elementIndex);
            if (element == "position2d") {
                positionOffset = offset;
                positionSize = 2;
                offset += positionSize * sizeof(float);
            } else if (element == "position3d" || element == "position") {
                positionOffset = offset;
                positionSize = 3;
                offset += positionSize * sizeof(float);
            } else if (element == "position4d") {
                positionOffset = offset;
                positionSize = 4;
                offset += positionSize * sizeof(float);
            } else if (element == "normal") {
                normalOffset = offset;
                offset += 3 * sizeof(float);
            } else if (element == "texcoord1d") {
                texcoordOffset = offset;
                texcoordSize = 1;
                offset += texcoordSize * sizeof(float);
            } else if (element == "texcoord2d" || element == "texcoord") {
                texcoordOffset = offset;
                texcoordSize = 2;
                offset += texcoordSize * sizeof(float);
            } else if (element == "texcoord3d") {
                texcoordOffset = offset;
                texcoordSize = 3;
                offset += texcoordSize * sizeof(float);
            } else if (element == "texcoord4d") {
                texcoordOffset = offset;
                texcoordSize = 4;
                offset += texcoordSize * sizeof(float);
            } else if (element == "rgb") {
                rgbOffset = offset;
                offset += 3 * sizeof(unsigned char);
            } else if (element == "rgba") {
                rgbaOffset = offset;
                offset += 4 * sizeof(unsigned char);
            }
            elementIndex = delimIndex + 1;
        }

        vertexLength = offset;
        buffer = new unsigned char[capacity * vertexLength];

        if (GLEW_VERSION_1_5)
            glGenBuffers(1, (GLuint*)&vbo_handle);
        else if (GLEW_ARB_vertex_buffer_object)
            glGenBuffersARB(1, (GLuint*)&vbo_handle);
//              cout << "[" << vbo_handle << "] new unsigned char[" << (capacity * vertexLength) << "]" << endl;
    }

    // Destructor
    Buffer::~Buffer()
    {
        cerr << "~Buffer " << this << endl;
        if (GLEW_VERSION_1_5)
            glDeleteBuffers(1, (GLuint*)&vbo_handle);
        else if (GLEW_ARB_vertex_buffer_object)
            glDeleteBuffersARB(1, (GLuint*)&vbo_handle);
        delete [] buffer;
    }

    // General methods
    void Buffer::invalidate()
    { valid = false; }
    void Buffer::validate()
    { valid = true; }
    bool Buffer::isValid()
    { return valid; }
    bool Buffer::isLoaded()
    { return loadedTo <= usedSpace(); }
    unsigned int Buffer::size()
    {
        return capacity * vertexLength;
    }
    unsigned int Buffer::usedSpace()
    { return length; }
    unsigned int Buffer::usedVertices()
    { return usedSpace() / vertexLength; }
    unsigned int Buffer::freeSpace()
    { return size() - length; }
    unsigned int Buffer::freeVertices()
    { return freeSpace() / vertexLength; }
    unsigned int Buffer::getVertexLength()
    { return vertexLength; }
    unsigned int Buffer::getVertexLengthFromFormat(string format)
    {
        unsigned int length = 0;
        size_t delimIndex = 0;
        size_t elementIndex = 0;
        while (delimIndex != string::npos) {
            delimIndex = format.find(':', elementIndex);
            string element = (delimIndex == string::npos) ? format.substr(elementIndex) : format.substr(elementIndex, delimIndex - elementIndex);
            if (element == "position2d")
                length += 2 * sizeof(float);
            else if (element == "position3d" || element == "position")
                length += 3 * sizeof(float);
            else if (element == "position4d")
                length += 4 * sizeof(float);
            else if (element == "normal")
                length += 3 * sizeof(float);
            else if (element == "texcoord1d")
                length += 1 * sizeof(float);
            else if (element == "texcoord2d" || element == "texcoord")
                length += 2 * sizeof(float);
            else if (element == "texcoord3d")
                length += 3 * sizeof(float);
            else if (element == "texcoord4d")
                length += 4 * sizeof(float);
            else if (element == "rgb")
                length += 3 * sizeof(unsigned char);
            else if (element == "rgba")
                length += 4 * sizeof(unsigned char);
            elementIndex = delimIndex + 1;
        }
        return length;
    }
    void Buffer::load()
    {
        valid = true;

        unsigned int size = usedSpace();

        if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
            if (loadedTo > 0) unload();
            loadedTo = size;
//                      cout << "[" << vbo_handle << "] glBufferData(," << size << ",,)" << endl;
            if (GLEW_VERSION_1_5) {
                glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
                glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_DYNAMIC_DRAW);
                int err;
                if ((err = glGetError())) fprintf(stderr, "c error %x\n", (unsigned int) err);
            } else if (GLEW_ARB_vertex_buffer_object) {
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_handle);
                glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, buffer, GL_DYNAMIC_DRAW);
                int err;
                if ((err = glGetError())) fprintf(stderr, "c error %x\n", (unsigned int) err);
            }
        }
    }
    void Buffer::load(unsigned int offset, unsigned int size)
    {
        if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
            if ((offset + size) * vertexLength > loadedTo)
                load();
            else {
                if (GLEW_VERSION_1_5) {
                    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
                    glBufferSubData(GL_ARRAY_BUFFER, offset * vertexLength, size * vertexLength, buffer + (offset * vertexLength));
                } else if (GLEW_ARB_vertex_buffer_object) {
                    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_handle);
                    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, offset * vertexLength, size * vertexLength, buffer + (offset * vertexLength));
                }
            }
        }
    }
    void Buffer::unload()
    {
        if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
            loadedTo = 0;
            if (GLEW_VERSION_1_5) {
                glDeleteBuffers(1, (GLuint*)&vbo_handle);
                glGenBuffers(1, (GLuint*)&vbo_handle);
            } else if (GLEW_ARB_vertex_buffer_object) {
                glDeleteBuffersARB(1, (GLuint*)&vbo_handle);
                glGenBuffersARB(1, (GLuint*)&vbo_handle);
            }
        }
    }

    // Data insertion methods
    void Buffer::setPosition(float x, float y, float z, float w)
    {
        *reinterpret_cast<float*>(&buffer[cursor + positionOffset]) = x;
        *reinterpret_cast<float*>(&buffer[cursor + positionOffset + sizeof(float)]) = y;
        if (positionSize > 2) *reinterpret_cast<float*>(&buffer[cursor + positionOffset + 2 * sizeof(float)]) = z;
        if (positionSize > 3) *reinterpret_cast<float*>(&buffer[cursor + positionOffset + 3 * sizeof(float)]) = w;
    }
    void Buffer::setNormal(float x, float y, float z)
    {
        *reinterpret_cast<float*>(&buffer[cursor + normalOffset]) = x;
        *reinterpret_cast<float*>(&buffer[cursor + normalOffset + sizeof(float)]) = y;
        *reinterpret_cast<float*>(&buffer[cursor + normalOffset + 2 * sizeof(float)]) = z;
    }
    void Buffer::setTexCoord(float x, float y, float z, float w)
    {
        *reinterpret_cast<float*>(&buffer[cursor + texcoordOffset]) = x;
        if (texcoordSize > 1) *reinterpret_cast<float*>(&buffer[cursor + texcoordOffset + sizeof(float)]) = y;
        if (texcoordSize > 2) *reinterpret_cast<float*>(&buffer[cursor + texcoordOffset + 2 * sizeof(float)]) = z;
        if (texcoordSize > 3) *reinterpret_cast<float*>(&buffer[cursor + texcoordOffset + 3 * sizeof(float)]) = w;
    }
    void Buffer::setColourf(float r, float g, float b, float a)
    { setColourb((unsigned char) (r * 255), (unsigned char) (g * 255), (unsigned char) (b * 255), (unsigned char) (a * 255)); }
    void Buffer::setColourb(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        if (rgbaOffset == -1) {
            buffer[cursor + rgbOffset] = r;
            buffer[cursor + rgbOffset + sizeof(unsigned char)] = g;
            buffer[cursor + rgbOffset + 2 * sizeof(unsigned char)] = b;
        } else {
            buffer[cursor + rgbaOffset] = r;
            buffer[cursor + rgbaOffset + sizeof(unsigned char)] = g;
            buffer[cursor + rgbaOffset + 2 * sizeof(unsigned char)] = b;
            buffer[cursor + rgbaOffset + 3 * sizeof(unsigned char)] = a;
        }
    }
    void Buffer::next()
    {
        cursor += vertexLength;
        if (cursor > length)
            length = cursor;
    }
    void Buffer::to(unsigned int index)
    {
        // cap index
        if (index >= capacity)
            index = capacity - 1;

        cursor = index * vertexLength;
        if (cursor > length)
            length = cursor;
    }
    bool Buffer::EOB()
    { return (cursor > (unsigned int) (vertexLength * (capacity - 1))); }

    // OpenGL methods
    bool Buffer::enable(unsigned int elements)
    {
        // Enable caching
        if (positionOffset >= 0 && elements & POSITION) glEnableClientState( GL_VERTEX_ARRAY );
        if (normalOffset >= 0 && elements & NORMAL) glEnableClientState( GL_NORMAL_ARRAY );
        if (texcoordOffset >= 0 && elements & TEXCOORD) glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        if ((rgbOffset >= 0 || rgbaOffset >= 0) && elements & COLOUR) glEnableClientState( GL_COLOR_ARRAY );

        // Bind cache
        if (GLEW_VERSION_1_5 || GLEW_ARB_vertex_buffer_object) {
            if (cursor > loadedTo) load();
            if (GLEW_VERSION_1_5)
                glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
            else
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_handle);
            if (positionOffset >= 0 && elements & POSITION) glVertexPointer(positionSize, GL_FLOAT, vertexLength, reinterpret_cast<void *>(positionOffset));
            if (normalOffset >= 0 && elements & NORMAL) glNormalPointer(GL_FLOAT, vertexLength, reinterpret_cast<void *>(normalOffset));
            if (texcoordOffset >= 0 && elements & TEXCOORD) glTexCoordPointer(texcoordSize, GL_FLOAT, vertexLength, reinterpret_cast<void *>(texcoordOffset));
            if (rgbOffset >= 0 && elements & COLOUR) glColorPointer(3, GL_UNSIGNED_BYTE, vertexLength, reinterpret_cast<void *>(rgbOffset));
            if (rgbaOffset >= 0 && elements & COLOUR) glColorPointer(4, GL_UNSIGNED_BYTE, vertexLength, reinterpret_cast<void *>(rgbaOffset));
        } else {
            if (positionOffset >= 0 && elements & POSITION) glVertexPointer(positionSize, GL_FLOAT, vertexLength, buffer + positionOffset);
            if (normalOffset >= 0 && elements & NORMAL) glNormalPointer(GL_FLOAT, vertexLength, buffer + normalOffset);
            if (texcoordOffset >= 0 && elements & TEXCOORD) glTexCoordPointer(texcoordSize, GL_FLOAT, vertexLength, buffer + texcoordOffset);
            if (rgbOffset >= 0 && elements & COLOUR) glColorPointer(3, GL_UNSIGNED_BYTE, vertexLength, buffer + rgbOffset);
            if (rgbaOffset >= 0 && elements & COLOUR) glColorPointer(4, GL_UNSIGNED_BYTE, vertexLength, buffer + rgbaOffset);
        }

        return true;
    }
    bool Buffer::disable()
    {
        // Disable caching
        if (positionOffset >= 0) glDisableClientState( GL_VERTEX_ARRAY );
        if (normalOffset >= 0) glDisableClientState( GL_NORMAL_ARRAY );
        if (texcoordOffset >= 0) glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        if (rgbOffset >= 0 || rgbaOffset >= 0) glDisableClientState( GL_COLOR_ARRAY );

        return true;
    }
    void Buffer::render(unsigned int mode, int first, int count)
    {
        // Only valid and loaded buffers can be rendered
        if (count == -1) count = usedVertices();
//              cout << "[" << vbo_handle << "] glDrawArrays(" << mode << ", " << first << ", " << count << ")" << endl;
        int err;
        glDrawArrays(mode, first, count);
        if ((err = glGetError())) fprintf(stderr, "c error %x\n", (unsigned int) err);
    }

} // namespace AMBROSIA
