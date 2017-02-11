###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    Utopia Documents is free software: you can redistribute it and/or modify
#    it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
#    published by the Free Software Foundation.
#    
#    Utopia Documents is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
#    
#    In addition, as a special exception, the copyright holders give
#    permission to link the code of portions of this program with the OpenSSL
#    library under certain conditions as described in each individual source
#    file, and distribute linked combinations including the two.
#    
#    You must obey the GNU General Public License in all respects for all of
#    the code used other than OpenSSL. If you modify file(s) with this
#    exception, you may extend this exception to your version of the file(s),
#    but you are not obligated to do so. If you do not wish to do so, delete
#    this exception statement from your version.
#    
#    You should have received a copy of the GNU General Public License
#    along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
#   
###############################################################################

include_directories (
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
)

SET(SOURCES
  shader.cpp
  vertexbuffer.cpp
  )

if(WIN32)
  add_utopia_library( gtl SHARED ${SOURCES} )
else()
  add_utopia_library( gtl STATIC ${SOURCES} )
endif()

target_link_libraries(gtl ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

# SET(HEADERS_CORE
#   algorithm.h
#   color.h
#   common.h
#   complex.h
#   config.h
#   extent.h
#   extrusion.h
#   functional.h
#   gl.h
#   gtl.h
#   interpolation.h
#   matrix.h
#   orientation.h
#   quaternion.h
#   scale.h
#   shader.h
#   shader2.h
#   triple.h
#   types.h
#   vector.h
#   vertexbuffer.h
#   vertexbuffer.new.h
# )

# SET(HEADERS_BITS
#   bits/matrix2.h
#   bits/matrix3.h
#   bits/matrix4.h
#   bits/orientation2.h
#   bits/orientation3.h
#   bits/rgb_swizzles.h
#   bits/rgba_swizzles.h
#   bits/scale2.h
#   bits/scale3.h
#   bits/vector2.h
#   bits/vector3.h
#   bits/vector4.h
#   bits/xy_swizzles.h
#   bits/xyz_swizzles.h
#   bits/xyzw_swizzles.h
# )

# SET(HEADERS_GL
#   gl/matrix.h
#   gl/rotation.h
#   gl/scale.h
#   gl/types.h
#   gl/vertex.h
# )

# SET(HEADERS_COLOR
#   color/CMY.h
#   color/HSV.h
#   color/Lab.h
#   color/RGB.h
#   color/spaces.h
#   color/XYZ.h
#   color/YIQ.h
#   color/YUV.h
# )
