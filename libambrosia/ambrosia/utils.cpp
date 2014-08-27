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

#include "ambrosia/utils.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

    bool setup = false;
    bool macDriverWorkaround = true;

    void OpenGLSetup()
    {
        if (setup) return;

        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            exit(1);
        }

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program) && defined(GL_ARB_shader_objects) && defined(GL_ARB_shading_language_100) && defined(GL_VERSION_2_0)
        // Fix MAC OS X driver problems
        if (!glCreateShader && glCreateShaderObjectARB) {
#ifdef __WIN32
            glCreateShader = glCreateShaderObjectARB;
#else
            glCreateShader = (GLuint (*)(GLenum))glCreateShaderObjectARB;
#endif
            macDriverWorkaround = true;
        }
#ifdef __WIN32
        if (!glShaderSource && glShaderSourceARB)
            glShaderSource = glShaderSourceARB;
        if (!glCompileShader && glCompileShaderARB)
            glCompileShader = glCompileShaderARB;
        if (!glGetShaderiv && glGetObjectParameterivARB)
            glGetShaderiv = glGetObjectParameterivARB;
        if (!glGetShaderInfoLog && glGetInfoLogARB)
            glGetShaderInfoLog = glGetInfoLogARB;
        if (!glCreateProgram && glCreateProgramObjectARB)
            glCreateProgram = glCreateProgramObjectARB;
        if (!glDetachShader && glDetachObjectARB)
            glDetachShader = glDetachObjectARB;
        if (!glDeleteProgram && glDeleteObjectARB)
            glDeleteProgram = glDeleteObjectARB;
        if (!glUseProgram && glUseProgramObjectARB)
            glUseProgram = glUseProgramObjectARB;
#else
        if (!glShaderSource && glShaderSourceARB)
            glShaderSource = (void (*)(GLuint, GLsizei, const GLchar**, const GLint*))glShaderSourceARB;
        if (!glCompileShader && glCompileShaderARB)
            glCompileShader = (void (*)(GLuint))glCompileShaderARB;
        if (!glGetShaderiv && glGetObjectParameterivARB)
            glGetShaderiv = (void (*)(GLuint, GLenum, GLint*))glGetObjectParameterivARB;
        if (!glGetShaderInfoLog && glGetInfoLogARB)
            glGetShaderInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*))glGetInfoLogARB;
        if (!glCreateProgram && glCreateProgramObjectARB)
            glCreateProgram = (GLuint (*)())glCreateProgramObjectARB;
        if (!glDetachShader && glDetachObjectARB)
            glDetachShader = (void (*)(GLuint, GLuint))glDetachObjectARB;
        if (!glDeleteProgram && glDeleteObjectARB)
            glDeleteProgram = (void (*)(GLuint))glDeleteObjectARB;
        if (!glUseProgram && glUseProgramObjectARB)
            glUseProgram = (void (*)(GLuint))glUseProgramObjectARB;
        if (!glGetUniformLocation && glGetUniformLocationARB)
            glGetUniformLocation = (GLint (*)(GLuint, const GLchar*)) glGetUniformLocationARB;
#endif
        if (!glUniform1f && glUniform1fARB)
            glUniform1f = glUniform1fARB;
        if (!glUniform2f && glUniform2fARB)
            glUniform2f = glUniform2fARB;
        if (!glUniform3f && glUniform3fARB)
            glUniform3f = glUniform3fARB;
        if (!glUniform4f && glUniform4fARB)
            glUniform4f = glUniform4fARB;
        if (!glUniform1fv && glUniform1fvARB)
            glUniform1fv = glUniform1fvARB;
        if (!glUniform2fv && glUniform2fvARB)
            glUniform2fv = glUniform2fvARB;
        if (!glUniform3fv && glUniform3fvARB)
            glUniform3fv = glUniform3fvARB;
        if (!glUniform4fv && glUniform4fvARB)
            glUniform4fv = glUniform4fvARB;
        if (!glUniformMatrix2fv && glUniformMatrix2fvARB)
            glUniformMatrix2fv = glUniformMatrix2fvARB;
        if (!glUniformMatrix3fv && glUniformMatrix3fvARB)
            glUniformMatrix3fv = glUniformMatrix3fvARB;
        if (!glUniformMatrix4fv && glUniformMatrix4fvARB)
            glUniformMatrix4fv = glUniformMatrix4fvARB;
        if (!glUniform1i && glUniform1iARB)
            glUniform1i = glUniform1iARB;
        if (!glUniform2i && glUniform2iARB)
            glUniform2i = glUniform2iARB;
        if (!glUniform3i && glUniform3iARB)
            glUniform3i = glUniform3iARB;
        if (!glUniform4i && glUniform4iARB)
            glUniform4i = glUniform4iARB;
        if (!glUniform1iv && glUniform1ivARB)
            glUniform1iv = glUniform1ivARB;
        if (!glUniform2iv && glUniform2ivARB)
            glUniform2iv = glUniform2ivARB;
        if (!glUniform3iv && glUniform3ivARB)
            glUniform3iv = glUniform3ivARB;
        if (!glUniform4iv && glUniform4ivARB)
            glUniform4iv = glUniform4ivARB;
#ifdef __WIN32
        if (!glLinkProgram && glLinkProgramARB)
            glLinkProgram = glLinkProgramARB;
        if (!glGetProgramiv && glGetObjectParameterivARB)
            glGetProgramiv = glGetObjectParameterivARB;
        if (!glGetProgramInfoLog && glGetInfoLogARB)
            glGetProgramInfoLog = glGetInfoLogARB;
        if (!glAttachShader && glAttachObjectARB)
            glAttachShader = glAttachObjectARB;
#else
        if (!glLinkProgram && glLinkProgramARB)
            glLinkProgram = (void (*)(GLuint))glLinkProgramARB;
        if (!glGetProgramiv && glGetObjectParameterivARB)
            glGetProgramiv = (void (*)(GLuint, GLenum, GLint*))glGetObjectParameterivARB;
        if (!glGetProgramInfoLog && glGetInfoLogARB)
            glGetProgramInfoLog = (void (*)(GLuint, GLsizei, GLsizei*, GLchar*))glGetInfoLogARB;
        if (!glAttachShader && glAttachObjectARB)
            glAttachShader = (void (*)(GLuint, GLuint))glAttachObjectARB;
#endif
#endif
#if defined(GL_ARB_vertex_buffer_object) && defined(GL_VERSION_1_5)
        if (!glGenBuffers && glGenBuffersARB)
            glGenBuffers = glGenBuffersARB;
        if (!glDeleteBuffers && glDeleteBuffersARB)
            glDeleteBuffers = glDeleteBuffersARB;
        if (!glBindBuffer && glBindBufferARB)
            glBindBuffer = glBindBufferARB;
        if (!glBufferData && glBufferDataARB)
            glBufferData = glBufferDataARB;
        if (!glBufferSubData && glBufferSubDataARB)
            glBufferSubData = glBufferSubDataARB;
        if (!glDeleteBuffers && glDeleteBuffersARB)
            glDeleteBuffers = glDeleteBuffersARB;
#endif

        setup = true;
    }

#ifdef __cplusplus
}
#endif
