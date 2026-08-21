# NPP SDK - CMake Build Configuration
# 
# Supports: GCC, Clang, MSVC
# Platforms: Linux, macOS, Windows, MCU

cmake_minimum_required(VERSION 3.16)
project(npp-sdk VERSION 1.0.260821.1 LANGUAGES C)

#=============================================================================
# Platform Detection
#=============================================================================

if(MSVC)
    message(STATUS "Compiler: MSVC")
    set(PLATFORM_WINDOWS TRUE)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
    set(PLATFORM_UNIX TRUE)
else()
    message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
endif()

#=============================================================================
# Build Options
#=============================================================================

# Optimization
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

#=============================================================================
# Compiler-Specific Flags (global)
#=============================================================================

if(PLATFORM_UNIX)
    # GCC/Clang specific flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0")
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
elseif(PLATFORM_WINDOWS)
    # MSVC specific flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi-")
endif()

#=============================================================================
# Obfuscation Options (O-LLVM)
#=============================================================================

# O-LLVM obfuscation (requires O-LLVM installation)
# Install: https://github.com/obfuscator-llvm/obfuscator
#
# set(OBFUSCATION_FLAGS
#     -mllvm -flatten      # Control flow flattening
#     -mllvm -bcf          # Bogus control flow
#     -mllvm -sub          # Instruction substitution
#     -mllvm -sobf         # String obfuscation
# )
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBFUSCATION_FLAGS}")

#=============================================================================
# Library Target
#=============================================================================

# Note: Source files (.c) are not in the public repository
# This is a reference build configuration

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

# Static library
add_library(npp_static STATIC ${NPP_SOURCES})
set_target_properties(npp_static PROPERTIES
    OUTPUT_NAME npp
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Header paths (internal use)
target_include_directories(npp_static PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
)

#=============================================================================
# Compiler-Specific Flags (target-specific, must be after add_library)
#=============================================================================

if(PLATFORM_UNIX)
    target_compile_options(npp_static PRIVATE
        -fno-rtti
        -fno-exceptions
        -fvisibility=hidden
    )
elseif(PLATFORM_WINDOWS)
    target_compile_options(npp_static PRIVATE
        /GR-    # Disable RTTI
        /EHsc-  # Disable exceptions
        /GL-    # Disable whole program optimization
    )
endif()

#=============================================================================
# Strip Symbols (Unix only)
#=============================================================================

if(PLATFORM_UNIX)
    add_custom_command(TARGET npp_static POST_BUILD
        COMMAND ${CMAKE_STRIP} -S $<TARGET_FILE:npp_static>
        COMMAND ${CMAKE_STRIP} -x $<TARGET_FILE:npp_static>
        COMMENT "Stripping symbols from libnpp.a..."
    )
elseif(PLATFORM_WINDOWS)
    # MSVC: Use /PDBSTRIPPED or similar
    message(STATUS "Note: Use 'lib /NOLOGO /NODEFAULTLIB' for Windows symbol stripping")
endif()

#=============================================================================
# Install
#=============================================================================

install(TARGETS npp_static
    ARCHIVE DESTINATION lib
)

install(FILES include/npp.h
    DESTINATION include
)
