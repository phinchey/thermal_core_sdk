cmake_minimum_required(VERSION 3.1)
project(sample
  LANGUAGES CXX
)


set(SRC_LIST
	camera.cpp
	cmd.cpp
	data.cpp
	display.cpp
	sample.cpp	
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/include
)


link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs)

add_executable(sample ${SRC_LIST})
target_link_libraries(sample
    iruvc
    iri2c
    ircmd
    irprocess
    irparse
    pthread

#opencv related
    opencv_highgui
    opencv_imgcodecs
    opencv_imgproc 
    opencv_core
)
