cmake_minimum_required(VERSION 3.19)

project(SerialTool
	VERSION 1.2.3
	LANGUAGES CXX
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS 
	Core Gui Widgets 
	SerialPort
	Network
)

qt_standard_project_setup()

if(CMAKE_BUILD_TYPE STREQUAL Release)
	set(EXE_FLAGS WIN32)			# windows 平台关闭终端显示
else()
	set(EXE_FLAGS "")
endif()

qt_add_executable(${CMAKE_PROJECT_NAME}
	${EXE_FLAGS}     

	src/main.cpp
	src/mainwindow.cpp
	src/appconfig.cpp
	src/networkmanager.cpp
	src/serialmanager.cpp
	src/dialog/sendfiledialog.cpp
	src/dialog/serialsettingdialog.cpp
	src/dialog/sendw25qxx.cpp
	src/tab_page/tabpage.cpp
	src/tab_page/list_item/customitem.cpp

	assets/assets.qrc
	assets/app_ico.rc
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
	src
	src/dialog
	src/tab_page
	src/tab_page/list_item
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
	APP_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	Qt::Core
	Qt::Gui
	Qt::Widgets
	Qt::SerialPort
	Qt::Network
)
