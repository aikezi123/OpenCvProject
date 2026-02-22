find_package(Qt6 COMPONENTS CORE REQUIRED)

add_library(VideoProcess STATIC
	src/VideoProcess.cpp
	include/VideoProcess.h
)

target_include_directories(VideoProcess 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(VideoProcess
	PUBLIC Qt6::Core
	CommonLogger
)