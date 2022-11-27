project(IIModManager_GUI VERSION ${IIMODMAN_VERSION})

set(IIMODMAN_GUI_SOURCES
    applypreviewcommand.cpp
    cacheaddcommand.cpp
    cacheimportinstalledcommand.cpp
    cacheimportmodel.cpp
    cachesavecommand.cpp
    cacheupdatecommand.cpp
    guiconfig.cpp
    guidownloader.cpp
    main.cpp
    mainwindow.cpp
    markupdatescommand.cpp
    modelutil.cpp
    modsmodel.cpp
    modspecpreviewmodel.cpp
    modmanguiapplication.cpp
    modssortfilterproxymodel.cpp
    openspecfilecommand.cpp
    settingsdialog.cpp
    util.cpp

    icon.qrc
  )

add_executable(${IIMODMAN_GUI_TARGET_NAME} ${IIMODMAN_GUI_SOURCES})
target_compile_definitions(${IIMODMAN_GUI_TARGET_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
    IIMODMAN_VERSION="${IIMODMAN_VERSION}"
  )
target_link_libraries(${IIMODMAN_GUI_TARGET_NAME}
    ${IIMODMAN_LIB_TARGET_NAME}
    ${IIMODMAN_GUI_QT_LIBRARIES}
  )

if(FLATPAK)
  install(TARGETS ${IIMODMAN_GUI_TARGET_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
  add_subdirectory(icons)
endif()