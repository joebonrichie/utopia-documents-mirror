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

#ifndef GTL_SHADER_INCL_
#define GTL_SHADER_INCL_

#include <gtl/config.h>
#include <string>
#include <list>
#include <iostream>

namespace gtl {

    class Shader {

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
        Shader(std::string, unsigned int);
        // Destructor
        ~Shader();

        // OpenGL methods
        static unsigned int capability();

    private:
        // Shader
        unsigned int shaderLanguage;
        std::string source;
        unsigned int shaderType;
        bool enabled;

        // OpenGL handles
        unsigned int shader;

        // friends
        friend class ShaderProgram;

    }; // class Shader

    // Utility functions to load a shader from a file or stream
    Shader * loadShader(std::string, unsigned int);
    Shader * loadShader(std::istream &, unsigned int);

    //
    // ShaderProgram class
    //

    class ShaderProgram {

    public:
        // Constructor
        ShaderProgram();
        // Destructor
        ~ShaderProgram();

        // OpenGL methods
        bool addShader(Shader *);
        bool addShader(std::string, unsigned int);
        bool enable();
        bool disable();
        static unsigned int capability();

        // Shader Variables
        int getUniformLocation(std::string);
        bool setUniformf(int, int, float = 0.0, float = 0.0, float = 0.0, float = 0.0);
        bool setUniformf(std::string, int, float = 0.0, float = 0.0, float = 0.0, float = 0.0);
        bool setUniformfv(int, int, float *);
        bool setUniformfv(std::string, int, float *);
        bool setUniformMatrixfv(int, int, unsigned char, float *);
        bool setUniformMatrixfv(std::string, int, unsigned char, float *);
        bool setUniformi(int, int, int = 0, int = 0, int = 0, int = 0);
        bool setUniformi(std::string, int, int = 0, int = 0, int = 0, int = 0);
        bool setUniformiv(int, int, int *);
        bool setUniformiv(std::string, int, int *);

    private:
        // Shaders
        std::list< Shader * > shaders;
        bool enabled;
        bool linked;

        // OpenGL handles
        unsigned int program;

        // OpenGL mehods
        bool link();

    }; // class ShaderProgram

} // namespace gtl

#endif /* GTL_SHADER_INCL_ */
