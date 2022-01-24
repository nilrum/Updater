cmake_minimum_required(VERSION 3.20)
project(Updater)

set(CMAKE_CXX_STANDARD 17)

include(C:/CMakeProjects/Fetch.cmake)
set(NAME_UPDATER VersionControl)

set(Updater_SRC Updater.cpp )

if(${QT_GUI})
    set(Updater_SRC ${Updater_SRC} Qt/InteractiveQt.cpp)
else()
    set(Updater_SRC ${Updater_SRC} InteractiveConsole.cpp)
endif()

add_executable(${NAME_UPDATER} main.cpp ${Updater_SRC})
target_include_directories(${NAME_UPDATER} PUBLIC ${FETCH_DIR} )
target_link_libraries(${NAME_UPDATER} PUBLIC Property cpr::cpr)

if(${QT_GUI})
    target_include_directories(${NAME_UPDATER} PUBLIC ${Qt5Widgets_INCLUDE_DIRS})
    target_link_libraries(${NAME_UPDATER} PUBLIC GuiLib)
    set(INTERACTIVE "\"Qt/InteractiveQt.h\"")
    target_compile_definitions(${NAME_UPDATER} PUBLIC INTERACTIVE=${INTERACTIVE})
    set_property(TARGET ${NAME_UPDATER} PROPERTY WIN32_EXECUTABLE true)
endif()