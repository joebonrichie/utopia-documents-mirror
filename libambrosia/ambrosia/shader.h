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

#ifndef AMBROSIA_SHADER_H
#define AMBROSIA_SHADER_H

#include <ambrosia/config.h>
#include <utopia2/utopia2.h>

#include <string>
#include <list>
#include <iostream>
using namespace std;

namespace AMBROSIA {

    //
    // Shader class
    //

    class LIBAMBROSIA_API Shader {

    public:
        // Typedefs
        enum {
            VERTEX = 0,
            FRAGMENT = 1
        };
        enum {
            NONE = 0,
            GLSL = 1
        };

        // Constructor
        Shader(string, unsigned int);
        // Destructor
        ~Shader();

        // OpenGL methods
        static unsigned int capability();

    private:
        // Shader
        unsigned int shaderLanguage;
        string source;
        unsigned int shaderType;
        bool enabled;

        // OpenGL handles
        unsigned int shader;

        // friends
        friend class ShaderProgram;

    }; // class Shader

    // Utility functions to load a shader from a file or stream
    LIBAMBROSIA_EXPORT Shader * loadShader(string, unsigned int);
    LIBAMBROSIA_EXPORT Shader * loadShader(istream &, unsigned int);

    //
    // ShaderProgram class
    //

    class LIBAMBROSIA_API ShaderProgram {

    public:
        // Constructor
        ShaderProgram();
        // Destructor
        ~ShaderProgram();

        // OpenGL methods
        bool addShader(Shader *);
        bool addShader(string, unsigned int);
        bool enable();
        bool disable();
        static unsigned int capability();

        // Shader Variables
        int getUniformLocation(string);
        bool setUniformf(int, int, float = 0.0, float = 0.0, float = 0.0, float = 0.0);
        bool setUniformf(string, int, float = 0.0, float = 0.0, float = 0.0, float = 0.0);
        bool setUniformfv(int, int, float *);
        bool setUniformfv(string, int, float *);
        bool setUniformMatrixfv(int, int, unsigned char, float *);
        bool setUniformMatrixfv(string, int, unsigned char, float *);
        bool setUniformi(int, int, int = 0, int = 0, int = 0, int = 0);
        bool setUniformi(string, int, int = 0, int = 0, int = 0, int = 0);
        bool setUniformiv(int, int, int *);
        bool setUniformiv(string, int, int *);

    private:
        // Shaders
        list< Shader * > shaders;
        bool enabled;
        bool linked;

        // OpenGL handles
        unsigned int program;

        // OpenGL mehods
        bool link();

    }; // class ShaderProgram

} // namespace AMBROSIA

#endif // AMBROSIA_SHADER_H
