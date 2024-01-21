cmake_minimum_required(VERSION 3.16)
project(YoloDetector)


# set(InferenceEngine_DIR "/opt/intel/openvino_2022.3.0/runtime/cmake")
# set(ngraph_DIR "/opt/intel/openvino_2022.3.0/runtime/cmake")

find_package(OpenCV REQUIRED)
find_package(InferenceEngine REQUIRED)
#message(STATUS "OpenVINO Dependence Libs is find. they are: ${InferenceEngine_DIR} ")
find_package(ngraph REQUIRED)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(YoloDetector main.cpp detector.cpp detector.hpp video_player.cpp video_player.hpp)

target_link_libraries(
        YoloDetector
        ${InferenceEngine_LIBRARIES}
        ${NGRAPH_LIBRARIES}
        ${OpenCV_LIBS}
)