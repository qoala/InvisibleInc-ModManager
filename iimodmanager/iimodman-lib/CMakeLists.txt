project(IIModManager_Library VERSION ${IIMODMAN_VERSION})

include(GNUInstallDirs) # configurable CMAKE_INSTALL_*DIR

set(IIMODMAN_LIB_HEADERS
    iimodman-lib_global.h
    mod.h
    moddownloader.h
    modlist.h
    modmanconfig.h
  )
set(IIMODMAN_LIB_SOURCES
    mod.cpp
    moddownloader.cpp
    modlist.cpp
    modmanconfig.cpp
  )

add_library(${IIMODMAN_LIB_TARGET_NAME} ${IIMODMAN_LIB_SOURCES})

set_target_properties(${IIMODMAN_LIB_TARGET_NAME} PROPERTIES
    VERSION ${IIMODMAN_VERSION}
    SOVERSION ${IIMODMAN_SOVERSION}
    PUBLIC_HEADER "${IIMODMAN_LIB_HEADERS}"
  )
target_compile_definitions(${IIMODMAN_LIB_TARGET_NAME} PRIVATE
    IIMODMANLIB_LIBRARY
    QT_DEPRECATED_WARNINGS
  )
target_include_directories(${IIMODMAN_LIB_TARGET_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/iimodman>
  )
target_link_libraries(${IIMODMAN_LIB_TARGET_NAME}
    Qt6::Core Qt6::Network
    QuaZip::QuaZip
  )