cmake_minimum_required(VERSION 2.8.11)
project(oasisReader)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
list( APPEND CMAKE_CXX_FLAGS "-std=c++1y ${CMAKE_CXX_FLAGS} -O3 -pipe -mtune=native ")

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
set(oasisreader_SRC
  src/main.cpp
  src/oasisreader.cpp
)


set(parser_SRC
# src/dataconstructs.cpp
# src/repetition.cpp
# src/delta.cpp
src/records.cpp
# src/pointlist.cpp
# src/layout.cpp
src/parser.cpp)

add_library(parser ${parser_SRC})
target_link_libraries(parser Qt5::Core)

enable_testing()
add_subdirectory(ut)




# Create code from a list of Qt designer ui files.
#set(CMAKE_AUTOUIC ON) # use this if you have CMake 3.x instead of the following
qt5_wrap_ui(oasisreader_SRC src/oasisreader.ui)

# Tell CMake to create the helloworld executable
add_executable(oasisreader ${oasisreader_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(oasisreader Qt5::Widgets)

# Install the executable
install(TARGETS oasisreader DESTINATION bin)
