cmake_minimum_required(VERSION 3.16)
project(WordLearningApp VERSION 2.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录
include_directories(include)

# 查找依赖库
find_package(PkgConfig REQUIRED)

# 查找httplib
find_path(HTTPLIB_INCLUDE_DIR httplib.h
    HINTS /usr/local/include /opt/homebrew/include /usr/include
)

# 查找nlohmann/json
find_path(JSON_INCLUDE_DIR nlohmann/json.hpp
    HINTS /usr/local/include /opt/homebrew/include /usr/include
)

if(NOT HTTPLIB_INCLUDE_DIR)
    message(FATAL_ERROR "httplib.h not found. Please install cpp-httplib")
endif()

if(NOT JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found. Please install nlohmann-json")
endif()

include_directories(${HTTPLIB_INCLUDE_DIR})
include_directories(${JSON_INCLUDE_DIR})

# 源文件
set(SOURCES
    src/main.cpp
    src/WordApp.cpp
    src/HttpServer.cpp
    src/FileUtils.cpp
    src/UserAuth.cpp
    src/UserDataManager.cpp
)

# 头文件
set(HEADERS
    include/WordApp.h
    include/HttpServer.h
    include/FileUtils.h
    include/UserAuth.h
    include/UserDataManager.h
    include/version.h
)

# 创建可执行文件
add_executable(word_app ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(word_app pthread)

# 设置输出目录
set_target_properties(word_app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# 编译选项
target_compile_options(word_app PRIVATE
    -Wall -Wextra -O2
    $<$<CONFIG:Debug>:-g -DDEBUG>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# 安装规则（生产环境）
install(TARGETS word_app
    RUNTIME DESTINATION /usr/local/bin
)

install(DIRECTORY data/
    DESTINATION /var/www/word-app/data
    FILES_MATCHING PATTERN "*.txt" PATTERN "*.json"
)

install(DIRECTORY assets/
    DESTINATION /var/www/word-app/assets
)

# 打包
set(CPACK_PACKAGE_NAME "WordLearningApp")
set(CPACK_PACKAGE_VERSION "2.0")
set(CPACK_PACKAGE_DESCRIPTION "Enterprise Word Learning Application")
set(CPACK_PACKAGE_CONTACT "word-learning-team@example.com")

include(CPack)
