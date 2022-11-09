project(IIModManager_GUI VERSION ${IIMODMAN_VERSION})

set(IIMODMAN_GUI_SOURCES
    cacheaddcommand.cpp
    cachesavecommand.cpp
    cachestatuscommand.cpp
    cacheupdatecommand.cpp
    installedstatuscommand.cpp
    installedupdatecommand.cpp
    installedsyncfilecommand.cpp
    main.cpp
    mainwindow.cpp
    modcachemodel.cpp
    modmanguiapplication.cpp
    settingsdialog.cpp
    util.cpp
  )

add_executable(${IIMODMAN_GUI_TARGET_NAME} ${IIMODMAN_GUI_SOURCES})
target_compile_definitions(${IIMODMAN_GUI_TARGET_NAME} PRIVATE
    QT_DEPRECATED_WARNINGS
  )
target_link_libraries(${IIMODMAN_GUI_TARGET_NAME}
    ${IIMODMAN_LIB_TARGET_NAME}
    ${IIMODMAN_GUI_QT_LIBRARIES})

