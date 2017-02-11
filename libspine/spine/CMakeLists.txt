###############################################################################
#   
#    This file is part of the libspine library.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    The libspine library is free software: you can redistribute it and/or
#    modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
#    VERSION 3 as published by the Free Software Foundation.
#    
#    The libspine library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
#    General Public License for more details.
#    
#    You should have received a copy of the GNU Affero General Public License
#    along with the libspine library. If not, see
#    <http://www.gnu.org/licenses/>
#   
###############################################################################

include_directories ( ${utf8_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${PCRE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} )

set(SOURCES
    Annotation.cpp
    Area.cpp
    Character.cpp
    Document.cpp
    TextSelection.cpp
    spineapi.cpp
    fingerprint.cpp
    )

add_definitions (
  -DUTF8PROC_EXPORTS
)

add_utopia_library(spine SHARED ${SOURCES})
target_link_libraries (spine utf8 ${PCRE_LIBRARIES} ${Boost_SIGNALS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${OPENSSL_LIBRARIES} )
install_utopia_library(spine "${COMPONENT}")
