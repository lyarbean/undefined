cmake_minimum_required(VERSION 3.2.0)
project(oasisReader)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
list( APPEND CMAKE_CXX_FLAGS "-std=c++1y ${CMAKE_CXX_FLAGS} -O3 -pipe -mtune=native ")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Quick REQUIRED)

set(oasisreader_SRC
  src/main.cpp
  src/beancellitem.cpp
  src/renderhelper.cpp
)


set(parser_SRC
src/records.cpp
src/parser.cpp)

add_library(parser ${parser_SRC})
target_link_libraries(parser Qt5::Core)

enable_testing()
add_subdirectory(ut)



add_executable(oasisreader ${oasisreader_SRC})


target_link_libraries(oasisreader Qt5::Widgets Qt5::Quick)


install(TARGETS oasisreader DESTINATION bin)
