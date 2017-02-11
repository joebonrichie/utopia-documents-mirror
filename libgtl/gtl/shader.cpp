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

#include <gtl/shader.h>
#include <gtl/gl.h>
#include <fstream>
using namespace std;

namespace gtl {


    Shader::Shader(string source, unsigned int shaderType)
        : shaderLanguage(NONE), source(source), shaderType(shaderType), enabled(false)
    {
        if (capability() == GLSL) {
            shaderLanguage = GLSL;
            if (GLEW_VERSION_2_0)
                shader = glCreateShader((shaderType == FRAGMENT) ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
            else
                shader = glCreateShaderObjectARB((shaderType == FRAGMENT) ? GL_FRAGMENT_SHADER_ARB : GL_VERTEX_SHADER_ARB);
            const char * c_source = source.c_str();
            if (GLEW_VERSION_2_0) {
                glShaderSource(shader, 1, (const GLchar **) &c_source, 0);
                glCompileShader(shader);
            } else {
                glShaderSourceARB(shader, 1, (const GLchar **) &c_source, 0);
                glCompileShaderARB(shader);
            }
            GLint value = 0;
            if (GLEW_VERSION_2_0)
                glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
            else
                glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &value);
            if (value != GL_TRUE) {
                cerr << "Error compiling shader" << endl;
                char logstr[4097] = {'\0'};
                if (GLEW_VERSION_2_0)
                    glGetShaderInfoLog(shader, 4096, 0, logstr);
                else
                    glGetInfoLogARB(shader, 4096, 0, logstr);
                cerr << logstr << endl;
            }
        }
    }

    // Destructor
    Shader::~Shader()
    {
        if (capability() == GLSL) {
            if (GLEW_VERSION_2_0)
                glDeleteShader(shader);
            else
                glDeleteObjectARB(shader);
        }
    }

    // OpenGL methods
    unsigned int Shader::capability()
    {
        if (GLEW_VERSION_2_0 || (GLEW_ARB_vertex_program && GLEW_ARB_fragment_program && GLEW_ARB_shader_objects && GLEW_ARB_shading_language_100))
            return GLSL;
        else
            return NONE;
    }

    // Utility functions to load a shader from a file or stream
    Shader * loadShader(string filename, unsigned int shaderType)
    {
        string source = "";
        ifstream stream(filename.c_str());
        if (!stream.is_open()) {
            string log = "Unable to load shader source file ";
            log += filename;
            return 0;
        }
        if (!stream.eof())
            return loadShader(stream, shaderType);
        else
            return 0;
    }
    Shader * loadShader(istream & stream, unsigned int shaderType)
    {
        string source = "";
        while (!stream.eof()) {
            string line = "";
            getline(stream, line);
            source += " " + line;
        }
        return new Shader(source, shaderType);
    }

    // Constructor
    ShaderProgram::ShaderProgram()
        : enabled(false), linked(false)
    {
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                program = glCreateProgram();
            else
                program = glCreateProgramObjectARB();
        }
    }

    // Destructor
    ShaderProgram::~ShaderProgram()
    {
        // Destroy all attached shaders
        for (list< Shader * >::iterator i = shaders.begin(); i != shaders.end(); ++i) {
            if (capability() == Shader::GLSL) {
                if (GLEW_VERSION_2_0)
                    glDetachShader(program, (*i)->shader);
                else
                    glDetachObjectARB(program, (*i)->shader);
            }
            delete (*i);
        }
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                glDeleteProgram(program);
            else
                glDeleteObjectARB(program);
        }
    }

    // OpenGL methods
    bool ShaderProgram::enable()
    {
        if (capability() == Shader::GLSL) {
            if (!linked) link();
            if (GLEW_VERSION_2_0)
                glUseProgram(program);
            else
                glUseProgramObjectARB(program);
        }
        return true;
    }
    bool ShaderProgram::disable()
    {
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                glUseProgram(0);
            else
                glUseProgramObjectARB(0);
        }
        return true;
    }
    int ShaderProgram::getUniformLocation(string name)
    {
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                return glGetUniformLocation(program, name.c_str());
            else
                return glGetUniformLocationARB(program, name.c_str());
        } else
            return -1;
    }
    bool ShaderProgram::setUniformf(int var, int count, float x, float y, float z, float w)
    {
        if (capability() == Shader::GLSL) {
            switch (count) {
            case 1:
                if (GLEW_VERSION_2_0)
                    glUniform1f(var, x);
                else
                    glUniform1fARB(var, x);
                break;
            case 2:
                if (GLEW_VERSION_2_0)
                    glUniform2f(var, x, y);
                else
                    glUniform2fARB(var, x, y);
                break;
            case 3:
                if (GLEW_VERSION_2_0)
                    glUniform3f(var, x, y, z);
                else
                    glUniform3fARB(var, x, y, z);
                break;
            case 4:
                if (GLEW_VERSION_2_0)
                    glUniform4f(var, x, y, z, w);
                else
                    glUniform4fARB(var, x, y, z, w);
            }
        }
        return true;
    }
    bool ShaderProgram::setUniformf(string name, int count, float x, float y, float z, float w)
    {
        if (capability() == Shader::GLSL) {
            int var = getUniformLocation(name);
            if (var == -1) return false;
            setUniformf(var, count, x, y, z, w);
        }
        return true;
    }
    bool ShaderProgram::setUniformfv(int var, int count, float * xyzw)
    {
        if (capability() == Shader::GLSL) {
            switch (count) {
            case 1:
                if (GLEW_VERSION_2_0)
                    glUniform1fv(var, count, xyzw);
                else
                    glUniform1fvARB(var, count, xyzw);
                break;
            case 2:
                if (GLEW_VERSION_2_0)
                    glUniform2fv(var, count, xyzw);
                else
                    glUniform2fvARB(var, count, xyzw);
                break;
            case 3:
                if (GLEW_VERSION_2_0)
                    glUniform3fv(var, count, xyzw);
                else
                    glUniform3fvARB(var, count, xyzw);
                break;
            case 4:
                if (GLEW_VERSION_2_0)
                    glUniform4fv(var, count, xyzw);
                else
                    glUniform4fvARB(var, count, xyzw);
            }
        }
        return true;
    }
    bool ShaderProgram::setUniformfv(string name, int count, float * xyzw)
    {
        if (capability() == Shader::GLSL) {
            int var = getUniformLocation(name);
            if (var == -1) return false;
            setUniformfv(var, count, xyzw);
        }
        return true;
    }
    bool ShaderProgram::setUniformMatrixfv(int var, int count, unsigned char transpose, float * values)
    {
        if (capability() == Shader::GLSL) {
            switch (count) {
            case 2:
                if (GLEW_VERSION_2_0)
                    glUniformMatrix2fv(var, count, transpose, values);
                else
                    glUniformMatrix2fvARB(var, count, transpose, values);
                break;
            case 3:
                if (GLEW_VERSION_2_0)
                    glUniformMatrix3fv(var, count, transpose, values);
                else
                    glUniformMatrix3fvARB(var, count, transpose, values);
                break;
            case 4:
                if (GLEW_VERSION_2_0)
                    glUniformMatrix4fv(var, count, transpose, values);
                else
                    glUniformMatrix4fvARB(var, count, transpose, values);
            }
        }
        return true;
    }
    bool ShaderProgram::setUniformMatrixfv(string name, int count, unsigned char transpose, float * values)
    {
        if (capability() == Shader::GLSL) {
            int var = getUniformLocation(name);
            if (var == -1) return false;
            setUniformMatrixfv(var, count, transpose, values);
        }
        return true;
    }
    bool ShaderProgram::setUniformi(int var, int count, int x, int y, int z, int w)
    {
        if (capability() == Shader::GLSL) {
            switch (count) {
            case 1:
                if (GLEW_VERSION_2_0)
                    glUniform1i(var, x);
                else
                    glUniform1iARB(var, x);
                break;
            case 2:
                if (GLEW_VERSION_2_0)
                    glUniform2i(var, x, y);
                else
                    glUniform2iARB(var, x, y);
                break;
            case 3:
                if (GLEW_VERSION_2_0)
                    glUniform3i(var, x, y, z);
                else
                    glUniform3iARB(var, x, y, z);
                break;
            case 4:
                if (GLEW_VERSION_2_0)
                    glUniform4i(var, x, y, z, w);
                else
                    glUniform4iARB(var, x, y, z, w);
            }
        }
        return true;
    }
    bool ShaderProgram::setUniformi(string name, int count, int x, int y, int z, int w)
    {
        if (capability() == Shader::GLSL) {
            int var = getUniformLocation(name);
            if (var == -1) return false;
            setUniformi(var, x, y, z, w);
        }
        return true;
    }
    bool ShaderProgram::setUniformiv(int var, int count, int * xyzw)
    {
        if (capability() == Shader::GLSL) {
            switch (count) {
            case 1:
                if (GLEW_VERSION_2_0)
                    glUniform1iv(var, count, (GLint*) xyzw);
                else
                    glUniform1ivARB(var, count, (GLint*) xyzw);
                break;
            case 2:
                if (GLEW_VERSION_2_0)
                    glUniform2iv(var, count, (GLint*) xyzw);
                else
                    glUniform2ivARB(var, count, (GLint*) xyzw);
                break;
            case 3:
                if (GLEW_VERSION_2_0)
                    glUniform3iv(var, count, (GLint*) xyzw);
                else
                    glUniform3ivARB(var, count, (GLint*) xyzw);
                break;
            case 4:
                if (GLEW_VERSION_2_0)
                    glUniform4iv(var, count, (GLint*) xyzw);
                else
                    glUniform4ivARB(var, count, (GLint*) xyzw);
            }
        }
        return true;
    }
    bool ShaderProgram::setUniformiv(string name, int count, int * xyzw)
    {
        if (capability() == Shader::GLSL) {
            int var = getUniformLocation(name);
            if (var == -1) return false;
            setUniformiv(var, count, xyzw);
        }
        return true;
    }
    bool ShaderProgram::link()
    {
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                glLinkProgram(program);
            else
                glLinkProgramARB(program);
            GLint value = 0;
            if (GLEW_VERSION_2_0)
                glGetProgramiv(program, GL_LINK_STATUS, &value);
            else
                glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &value);
            if (value != GL_TRUE) {
                cerr << "Error linking program" << endl;
                char logstr[4097] = {'\0'};
                if (GLEW_VERSION_2_0)
                    glGetProgramInfoLog(program, 4096, 0, logstr);
                else
                    glGetInfoLogARB(program, 4096, 0, logstr);
                cerr << logstr << endl;
            }
        }
        linked = true;
        return true;
    }
    bool ShaderProgram::addShader(Shader * shader)
    {
        if (shader == 0) return false;

        shaders.push_back(shader);
        if (capability() == Shader::GLSL) {
            if (GLEW_VERSION_2_0)
                glAttachShader(program, shader->shader);
            else
                glAttachObjectARB(program, shader->shader);
        }
        linked = false;
        return true;
    }
    bool ShaderProgram::addShader(string source, unsigned int shaderType)
    { return addShader(new Shader(source, shaderType)); }
    unsigned int ShaderProgram::capability()
    { return Shader::capability(); }

} // namespace gtl
