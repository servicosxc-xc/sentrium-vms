qt_add_executable(sentrium-client
    main.cpp
    DeviceController.cpp
    DeviceController.hpp
    DeviceWorker.cpp
    DeviceWorker.hpp
)

# Qt 6.12 qmlcachegen emits a CameraContextMenu_qml.cpp translation unit
# that triggers MSVC C4702 inside Qt headers with /W4 /WX on VS 17.14.
# Keep this single context menu as an embedded QML resource (loaded by
# the engine at runtime), preserving AOT compilation for every other QML.
# QT_QML_SKIP_CACHEGEN is the supported per-file Qt 6.12 property.
set_source_files_properties(CameraContextMenu.qml PROPERTIES
    QT_QML_SKIP_CACHEGEN TRUE
)

qt_add_qml_module(sentrium-client
    URI Sentrium.Client
    VERSION 1.0
    QML_FILES
        Main.qml
        HomeDashboard.qml
        DeviceExplorer.qml
        DevicesWorkspace.qml
        MosaicLayoutsPopup.qml
        CameraContextMenu.qml
)

target_link_libraries(sentrium-client PRIVATE
    sentrium-platform-win
    sentrium-intelbras
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

set_target_properties(sentrium-client PROPERTIES
    WIN32_EXECUTABLE TRUE
)
