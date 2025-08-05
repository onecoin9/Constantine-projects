# ThirdParty.cmake - 第三方库管理
# 该文件集中管理所有第三方库的引入和配置

# 设置第三方库的根目录
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# spdlog配置
option(USE_SPDLOG "使用spdlog日志库" ON)  # 重新启用spdlog
if(USE_SPDLOG)
    # 方式1: 使用FetchContent自动下载
    set(SPDLOG_SOURCE_DIR ${THIRD_PARTY_DIR}/spdlog)
    include(FetchContent)
    
    # 设置 Git 选项
    set(FETCHCONTENT_QUIET OFF)
    set(GIT_SHALLOW 1)
    
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.12.0
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(spdlog)
    
    # 方式2: 使用本地第三方库目录
    # add_subdirectory(${THIRD_PARTY_DIR}/spdlog)
    
    # 方式3: 使用系统安装的spdlog
    # find_package(spdlog REQUIRED)
endif()

# 其他第三方库配置示例
# option(USE_JSON "使用nlohmann/json库" ON)
# if(USE_JSON)
#     FetchContent_Declare(
#         json
#         GIT_REPOSITORY https://github.com/nlohmann/json.git
#         GIT_TAG        v3.11.2
#     )
#     FetchContent_MakeAvailable(json)
# endif()

# 创建一个接口库来管理所有第三方依赖
add_library(third_party_libs INTERFACE)

if(USE_SPDLOG)
    target_link_libraries(third_party_libs INTERFACE spdlog::spdlog)
    target_compile_definitions(third_party_libs INTERFACE USE_SPDLOG)
endif()

# if(USE_JSON)
#     target_link_libraries(third_party_libs INTERFACE nlohmann_json::nlohmann_json)
#     target_compile_definitions(third_party_libs INTERFACE USE_JSON)
# endif() 