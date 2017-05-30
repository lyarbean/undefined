cmake_minimum_required(VERSION 3.2.0)
project(oasisReader)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
list( APPEND CMAKE_CXX_FLAGS "-std=c++1y ${CMAKE_CXX_FLAGS}  -pipe -mtune=native -g -ggdb")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5Gui REQUIRED)

set(oasisreader_SRC
  main.cpp
  threadrenderer.cpp

#   src/beancellitem.cpp
#   src/renderhelper.cpp
)


set(parser_SRC
# src/records.cpp
src/mesh.cpp
src/parser.cpp)

add_library(parser ${parser_SRC})
target_link_libraries(parser Qt5::Core Qt5::Gui)

enable_testing()
add_subdirectory(ut)



set(Resources qml.qrc)
qt5_add_resources(RCS ${Resources})

add_executable(oasisreader ${oasisreader_SRC} ${RCS})


target_link_libraries(oasisreader Qt5::Widgets Qt5::Quick parser)


install(TARGETS oasisreader DESTINATION bin)
