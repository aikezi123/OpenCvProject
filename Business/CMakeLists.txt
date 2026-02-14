find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(Business STATIC
    src/TargetRecognition.cpp
    include/TargetRecognition.h
)

target_link_libraries(Business PUBLIC 
    Qt6::Core       # 业务逻辑通常需要 Qt 的信号槽机制
    Service         # 调用算法
)

target_include_directories(Business PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)