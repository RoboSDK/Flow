find_package( OpenCV REQUIRED )

list(APPEND libraries flow::flow CONAN_PKG::spdlog ${OpenCV_LIBS})

add_executable(video_capture video_capture.cpp)
target_link_libraries(video_capture PRIVATE ${libraries})
target_include_directories( video_capture PRIVATE ${OpenCV_INCLUDE_DIRS} )
