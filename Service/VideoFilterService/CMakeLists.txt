#强制添加OpenCV库的依赖，确保在编译时能够找到OpenCV的头文件和库文件。
find_package(OpenCV REQUIRED)

#将VideoFilterService独立生成一个静态库，包含其源文件和头文件。
add_library(VideoFilterService STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/VideoFilterService.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/VideoFilterService.h
)

target_include_directories(VideoFilterService
	PUBLIC ${CMAKE_CURRENT_DIRECTORY_DIR}/include
	PRIVATE ${CMAKE_CURRENT_DIRECTORY_DIR}/src
)

target_link_libraries(VideoFilterService
	PUBLIC 
	${OpenCV_LIBS} 
	VideoProcess
	CommonLogger
)

