# NPP SDK v2.0 - CMake Build Configuration
# 
# 本CMakeLists.txt用于构建混淆后的静态库
# 公开仓库不包含源代码，仅提供构建配置和预编译库

cmake_minimum_required(VERSION 3.16)
project(npp-sdk VERSION 2.0.0 LANGUAGES C)

#=============================================================================
# Build Options
#=============================================================================

# 优化级别：O2平衡性能和混淆
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# 移除所有调试信息
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0")

# 隐藏所有符号（默认不导出）
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

#=============================================================================
# Obfuscation Options (使用O-LLVM时取消注释)
#=============================================================================

# O-LLVM混淆选项（需要安装O-LLVM）
# 安装: https://github.com/obfuscator-llvm/obfuscator
#
# set(OBFUSCATION_FLAGS
#     -mllvm -flatten      # 控制流平坦化
#     -mllvm -bcf          # 虚假控制流
#     -mllvm -sub          # 指令替换
#     -mllvm -sobf         # 字符串加密
# )
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBFUSCATION_FLAGS}")

#=============================================================================
# Library Target
#=============================================================================

# 注意：源代码文件(.c)不放入公开仓库
# 这里仅作为构建配置的参考

set(NPP_SOURCES
    # src/core/npp_core.c
    # src/core/npp_error.c
    # src/core/npp_log.c
    # src/core/wake_pipe.c
    # src/pipe/pipe_scheduler.c
    # src/session/session_manager.c
    # src/gateway/npp_gateway.c
    # src/multicast/multicast.c
    # src/transport/udp.c
    # src/adaptive/adaptive.c
)

# 静态库
add_library(npp_static STATIC ${NPP_SOURCES})
set_target_properties(npp_static PROPERTIES
    OUTPUT_NAME npp
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# 编译选项
target_compile_options(npp_static PRIVATE
    -fno-rtti           # 禁用RTTI
    -fno-exceptions     # 禁用异常
    -fvisibility=hidden # 隐藏符号
)

# 头文件路径（内部使用）
target_include_directories(npp_static PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

#=============================================================================
# Strip Symbols (发布前执行)
#=============================================================================

# 剥离符号表
add_custom_command(TARGET npp_static POST_BUILD
    COMMAND ${CMAKE_STRIP} -S $<TARGET_FILE:npp_static>
    COMMAND ${CMAKE_STRIP} -x $<TARGET_FILE:npp_static>
    COMMENT "Stripping symbols from libnpp.a..."
)

#=============================================================================
# Install
#=============================================================================

install(TARGETS npp_static
    ARCHIVE DESTINATION lib
)

install(FILES include/npp.h
    DESTINATION include
)
