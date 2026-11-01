qt_add_executable(sentrium-client
    main.cpp
    DeviceController.cpp
    DeviceController.hpp
    DeviceWorker.cpp
    DeviceWorker.hpp
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
