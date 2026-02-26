# Infrastructure/Logger/CMakeLists.txt
add_library(CommonLogger SHARED
    include/CommonLogger.h
    src/CommonLogger.cpp
)

# 暴露头文件给其他所有层（因为所有层都需要打日志）
target_include_directories(CommonLogger 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)