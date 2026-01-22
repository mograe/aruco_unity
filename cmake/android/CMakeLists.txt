cmake_minimum_required(VERSION 3.22)
project(aruco_unity LANGUAGES C CXX)

add_library(aruco_unity SHARED 
	aruco_unity.cpp
)

set(OpenCV_DIR "$ENV{HOME}/build-opencv-arm64")

find_package(OpenCV REQUIRED COMPONENTS core imgproc calib3d aruco)

target_include_directories(aruco_unity PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(aruco_unity PRIVATE ${OpenCV_LIBS})

set_target_properties(aruco_unity PROPERTIES OUTPUT_NAME "aruco_unity")
