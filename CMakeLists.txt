cmake_minimum_required(VERSION 3.19)

project(SerialTool2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS 
    Core Gui Widgets 
    SerialPort
    Network
)

qt_standard_project_setup()

qt_add_executable(SerialTool2
    MACOSX_BUNDLE           # 启用qDebug()
    # WIN32 MACOSX_BUNDLE     # 关闭qDebug()

    src/main.cpp

    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui

    src/dataStructure.h

    src/appconfig.cpp
    src/appconfig.h

    src/networkmanager.cpp
    src/networkmanager.h

    src/serialmanager.cpp
    src/serialmanager.h

    src/dialog/sendfiledialog.cpp
    src/dialog/sendfiledialog.h
    src/dialog/sendfiledialog.ui

    src/dialog/serialsettingdialog.cpp
    src/dialog/serialsettingdialog.h
    src/dialog/serialsettingdialog.ui

    src/tab_page/tabpage.cpp
    src/tab_page/tabpage.h
    src/tab_page/tabpage.ui

    src/tab_page/list_item/customitem.cpp
    src/tab_page/list_item/customitem.h
    src/tab_page/list_item/customitem.ui

    assets/assets.qrc
    assets/app_ico.rc
)

target_include_directories(SerialTool2 PRIVATE src)

target_link_libraries(SerialTool2
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SerialPort
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS SerialTool2
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SerialTool2
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
