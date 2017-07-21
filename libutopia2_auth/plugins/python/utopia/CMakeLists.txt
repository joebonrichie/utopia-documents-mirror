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

INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
    ${utopia2_auth_INCLUDE_DIR}
    )

set(CMAKE_SWIG_OUTDIR "${CMAKE_BINARY_DIR}/${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SWIG_OUTDIR}")

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(utopiaauth.i PROPERTIES CPLUSPLUS ON)

SWIG_ADD_MODULE(utopiaauth python utopiaauth.i)

if(WIN32)
    # changes dll name to pyd
    SET_TARGET_PROPERTIES(_utopiaauth PROPERTIES SUFFIX ".pyd")
elseif(APPLE)
    SET_TARGET_PROPERTIES(_utopiaauth PROPERTIES LINK_FLAGS " -Wl,-F. -bundle -undefined dynamic_lookup")
endif()

install(FILES "${CMAKE_SWIG_OUTDIR}/utopiaauth.py" DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages" COMPONENT "${COMPONENT}")
if(APPLE)
    install(TARGETS _utopiaauth DESTINATION "${PLUGIN_INSTALL_DIR}" COMPONENT "${COMPONENT}")
    SWIG_LINK_LIBRARIES(utopiaauth utf8 utopia2_auth stdc++)
else()
    install(TARGETS _utopiaauth DESTINATION "${PYTHON_INSTALL_DIR}/lib/python${PYTHON_VERSION}/site-packages" COMPONENT "${COMPONENT}")
    SWIG_LINK_LIBRARIES(utopiaauth utf8 ${PYTHON_LIBRARIES} utopia2_auth stdc++)
endif()

ADD_DEPENDENCIES(build _utopiaauth)
