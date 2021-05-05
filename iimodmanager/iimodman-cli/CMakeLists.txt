project(IIModManager_CLI VERSION ${IIMODMAN_VERSION})

set(IIMODMAN_CLI_SOURCES
    addmodsimpl.cpp
    cachecommands.cpp
    cacheaddcommand.cpp
    cacheaddinstalledcommand.cpp
    cachelistcommand.cpp
    cacheupdatecommand.cpp
    confirmationprompt.cpp
    command.cpp
    commandcategory.cpp
    commandparser.cpp
    configcommands.cpp
    configgetcommand.cpp
    configlistcommand.cpp
    configsetcommand.cpp
    main.cpp
    modmancliapplication.cpp
    modscommands.cpp
    modsinstallcommand.cpp
    modslistcommand.cpp
    modssynccommand.cpp
    steamapicommands.cpp
    steamapimoddownloadcommand.cpp
    steamapimodinfocommand.cpp
    updatemodsimpl.cpp
  )

add_executable(${IIMODMAN_CLI_TARGET_NAME} ${IIMODMAN_CLI_SOURCES})
target_compile_definitions(${IIMODMAN_CLI_TARGET_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
  )
target_link_libraries(${IIMODMAN_CLI_TARGET_NAME}
    ${IIMODMAN_LIB_TARGET_NAME}
    ${IIMODMAN_LIB_QT_LIBRARIES})

