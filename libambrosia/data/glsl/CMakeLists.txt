###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2014 Lost Island Labs
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


project(Shaders)

set(SHADERS
    circle-outline.frag
    circle-outline.vert
    circle.frag
    circle.vert
    colour.frag
    colour.vert
    diffuse.frag
    diffuse.vert
    no_lighting.frag
    no_lighting.vert
    specular.frag
    specular.vert
    sphere-depth.frag
    sphere-depth.vert
    sphere.frag
    sphere.vert
)

foreach(SHADER ${SHADERS})
  configure_file(${SHADER} ${CMAKE_BINARY_DIR}/${RESOURCE_INSTALL_DIR}/ambrosia/glsl/${SHADER} COPYONLY)
  install(FILES  ${CMAKE_BINARY_DIR}/${RESOURCE_INSTALL_DIR}/ambrosia/glsl/${SHADER} DESTINATION ${RESOURCE_INSTALL_DIR}/ambrosia/glsl/ COMPONENT "${COMPONENT}")
endforeach()


