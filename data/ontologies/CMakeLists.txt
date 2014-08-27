
project(Ontologies)

set(ONTOLOGIES
    mygrid.nt
    bibo.nt
    owl.nt
    rdf.nt
    rdfs.nt
    utopia_domain.nt
    utopia_system.nt
)

foreach(ONT ${ONTOLOGIES})
  configure_file(${ONT} "${CMAKE_BINARY_DIR}/${RESOURCE_INSTALL_DIR}/ontologies/${ONT}" COPYONLY)
 install(FILES "${CMAKE_BINARY_DIR}/${RESOURCE_INSTALL_DIR}/ontologies/${ONT}" DESTINATION "${RESOURCE_INSTALL_DIR}/ontologies/" COMPONENT "${COMPONENT}")
endforeach()


