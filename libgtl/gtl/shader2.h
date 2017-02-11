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

#include <gtl/gl.h>
#include <string>

#include <vector>
#include <gtl/vector.h>

namespace gtl {

    /**
       \class GLSLShader
       \brief Base class for GLSL shaders, encapsulating the shader object and
       handling errors.
    */
    class GLSLShader {

        /**
         *  Destructor.
         */
        virtual ~GLSLShader()
            {
                if (GLSLShader::supported()) {
                    if (GLEW_VERSION_2_0) {
                        glDeleteShader(_shaderObject);
                    } else {
                        glDeleteObjectARB(_shaderObject);
                    }
                }
            }

        /**
         *  Is GLSL supported.
         */
        static bool supported() {
            return (GLEW_VERSION_2_0 ||
                    (GLEW_ARB_vertex_program &&
                     GLEW_ARB_fragment_program &&
                     GLEW_ARB_shader_objects &&
                     GLEW_ARB_shading_language_100));
        }

        /**
         *  Current error (if any).
         */
        std::string errorString() {
            return _errorString;
        }

        /**
         *  Is the shader object valid.
         */
        bool good() {
            return _shaderObject!=0;
        }

        /**
         *  Is the shader object invalid.
         */
        bool bad() {
            return _shaderObject==0;
        }


        /**
         *  Convert to the uint used by OpenGL to refer to this shader object.
         *  In the case of errors this will return 0.
         */
        operator uint() {
            return _shaderObject;
        }

    protected:

        /**
         *  Constructor.
         */
        GLSLShader(std::string source_, uint type_)
            : _shaderObject (0)
            {
                if(GLSLShader::supported()) {
                    const GLchar *c_source (source_.c_str());
                    char logstr[4097] = {'\0'};
                    GLint compile_status(0);

                    if (GLEW_VERSION_2_0) {
                        assert (type==GL_FRAGMENT_SHADER || type==GL_VERTEX_SHADER);

                        _shaderObject = glCreateShader(_type);
                        glShaderSource(_shaderObject, 1, &c_source, 0);
                        glCompileShader(_shaderObject);
                        glGetShaderiv(_shaderObject,
                                      GL_COMPILE_STATUS, &compile_status);

                        if (compile_status != GL_TRUE) {
                            glGetShaderInfoLog(shader, 4096, 0, logstr);
                            this->setError (logstr);
                        }

                    } else {

                        assert (type==GL_FRAGMENT_SHADER_ARB ||
                                type==GL_VERTEX_SHADER_ARB);
                        _shaderObject =
                            glCreateShaderObjectARB(_type);
                        glShaderSourceARB(_shaderObject, 1, &c_source, 0);
                        glCompileShaderARB(_shaderObject);
                        glGetObjectParameterivARB(_shaderObject,
                                                  GL_OBJECT_COMPILE_STATUS_ARB,
                                                  &compile_status);

                        if (compile_status != GL_TRUE) {
                            glGetInfoLogARB(shader, 4096, 0, logstr);
                            this->setError (logstr);
                        }

                    }

                } else {
                    setError ("Support for GLSL is unavailable.");
                }
            }

        /**
         *  \brief Internal method to set the error status and message.
         *  \note  This is for permanent errors.
         */
        void setError (const char *msg_) {
            _errorString=msg_;
            if (GLSLShader::supported()) {
                if (GLEW_VERSION_2_0)
                    glDeleteShader(_shaderObject);
                else
                    glDeleteObjectARB(_shaderObject);
            }
            _shaderObject=0;
        }

        uint _shaderObject;
        std::string _errorString;

    }; // class GLSLShader

    /**
       \class GLSLVertexShader
       \brief GLSL vertex shader.
    */
    class GLSLVertexShader : public GLSLShader {

    public:

        /**
         *  Constructor.
         */
        GLSLVertexShader(std::string source_)
            : GLSLShader(source_, GLEW_VERSION_2_0 ?
                         GL_VERTEX_SHADER : GL_VERTEX_SHADER_ARB)
            {}

        // Utility functions to load a shader from a file or stream
        static GLSLVertexShader * loadShader(string filename_)
            {
                std::ifstream stream(filename_.c_str());
                return GLSLVertexShader::loadShader(stream);
            }

        static GLSLVertexShader * loadShader(std::istream &stream_)
            {
                GLSLVertexShader *result (0);

                std::string source = "";
                if(stream.good()) {

                    while (!stream.eof()) {
                        string line = "";
                        getline(stream, line);
                        source += " " + line;
                    }
                    result=new GLSLVertexShader(source);
                }
                return result;
            }

    }; // class GLSLVertexShader

    /**
       \class GLSLFragmentShader
       \brief GLSL fragment shader.
    */
    class GLSLFragmentShader : public GLSLShader {

    public:

        /**
         *  Constructor.
         */
        GLSLFragmentShader(std::string source_)
            : GLSLShader(source_, GLEW_VERSION_2_0 ?
                         GL_FRAGMENT_SHADER : GL_FRAGMENT_SHADER_ARB)
            {}

        // Utility functions to load a shader from a file or stream
        static GLSLFragmentShader * loadShader(string filename_)
            {
                std::ifstream stream(filename_.c_str());
                return GLSLFragmentShader::loadShader(stream);
            }

        static GLSLFragmentShader * loadShader(std::istream &stream_)
            {
                GLSLFragmentShader *result (0);

                std::string source = "";
                if(stream.good()) {

                    while (!stream.eof()) {
                        string line = "";
                        getline(stream, line);
                        source += " " + line;
                    }
                    result=new GLSLFragmentShader(source);
                }
                return result;
            }

    }; // class GLSLFragmentShader

    class GLSLProgram {

    public:
        GLSLProgram();
        ~GLSLProgram();

        bool addShader(GLSLShader *shader_) {

        }

        bool addShader(std::string source_) {

        }

        bool setUniform(std::string uniform_, float arg_);
        bool setUniform(std::string uniform_, int arg_);

        bool setUniform(std::string uniform_, vector< float, 2 > arg_);
        bool setUniform(std::string uniform_, vector< int, 2 > arg_);

        bool setUniform(std::string uniform_, vector< float, 3 > arg_);
        bool setUniform(std::string uniform_, vector< int, 3 > arg_);

        bool setUniform(std::string uniform_, vector< float, 4 > arg_);
        bool setUniform(std::string uniform_, vector< int, 4 > arg_);

        bool setUniformf(std::string uniform_,
                         float arg1_, float arg2_, float arg3_);
        bool setUniformf(std::string uniform_,
                         float arg1_, float arg2_, float arg3_, float arg4_);

// TODO:
//      bool setUniform(std::string uniform_, matrix< float, 3 >, bool transpose=false);
//      bool setUniform(std::string uniform_, matrix< float, 4 >, bool transpose=false);

    private:

        bool link();

        std::vector< GLSLShader * > _shaders;
        bool _enabled;
        bool _linked;
        uint _programObject;


    }; // class GLSLProgram

} // namespace gtl

#endif // GTL_SHADER_INCL_
