cmake_minimum_required(VERSION 3.0)

project(NewApp)

# Set the path to your OpenCV installation directory
set(OpenCV_DIR "/usr/local/include/opencv4/")

# Set the path to your Nuitrack installation directory
set(NUITRACK_DIR "/usr/local")

# Include Nuitrack headers
include_directories(${NUITRACK_DIR}/lib/nuitrack)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add your source files to the project
add_executable(NewApp NewApp.cpp)

# Link OpenCV libraries
target_link_libraries(NewApp ${OpenCV_LIBS} -lnuitrack)

