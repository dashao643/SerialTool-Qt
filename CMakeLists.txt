cmake_minimum_required(VERSION 3.19)

# 此处修改项目名称
set(CMAKE_PROJECT_NAME SerialTool)

project(${CMAKE_PROJECT_NAME} 
	VERSION 1.2.1
	LANGUAGES CXX
)

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

qt_add_executable(${CMAKE_PROJECT_NAME}
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

	src/dialog/sendw25qxx.cpp
	src/dialog/sendw25qxx.h
	src/dialog/sendw25qxx.ui

	src/tab_page/tabpage.cpp
	src/tab_page/tabpage.h
	src/tab_page/tabpage.ui

	src/tab_page/list_item/customitem.cpp
	src/tab_page/list_item/customitem.h
	src/tab_page/list_item/customitem.ui

	assets/assets.qrc
	assets/app_ico.rc
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
	APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${CMAKE_PROJECT_NAME}
	PRIVATE
		Qt::Core
		Qt::Gui
		Qt::Widgets
		Qt::SerialPort
		Qt::Network
)

include(GNUInstallDirs)

install(TARGETS ${CMAKE_PROJECT_NAME}
	BUNDLE  DESTINATION .
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
	TARGET ${CMAKE_PROJECT_NAME}
	OUTPUT_SCRIPT deploy_script
	NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
