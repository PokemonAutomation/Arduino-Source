# Ccache.cmake: set up ccache to save compile time on mac/Linux.
#
# Included from CMakeLists.txt right after project() and before any target is created.
#
# Options, passed to CMake as -D<name>=<value>:
#   PA_USE_CCACHE          Use ccache on mac/Linux if it is installed (default OFF). Needs
#                          ccache 4.8+. Stops with an error if a compiler launcher is also
#                          given, e.g. -DCMAKE_CXX_COMPILER_LAUNCHER=ccache: use one or the other.
#   PA_CCACHE_CONFIG       ccache settings for every compile (default: Arduino-Source has
#                          its own ccache folder, and base_dir set to the checkout's root
#                          folder). base_dir allows builds to share the cache across worktrees.
#   PA_CCACHE_DEBUG_PATHS  Change to using relative paths in debug (Debug and RelWithDebInfo)
#                          builds (default ON) when ccache is on. This allows debug builds
#                          to also share the cache across worktrees.

option(PA_USE_CCACHE "Compile through ccache on mac/Linux if it is installed" OFF)

# set _pa_checkout_dir to be the repo root path
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH _pa_checkout_dir)

# By default, builds with debug info (Debug and RelWithDebInfo) store the build folder's
# full paths in it, so ccache can't share cache across different worktrees, as worktrees
# have their own full paths.
# When both PA_USE_CCACHE and PA_CCACHE_DEBUG_PATHS are on, we added option
# -fdebug-prefix-map <repo root path>
# -fdebug-compilation-dir=.
# as compiler flags. This lets the debug builds use relative paths, enabling cache sharing
# across worktrees.
option(PA_CCACHE_DEBUG_PATHS "With ccache, use relative paths in debug info so Debug and RelWithDebInfo builds share the cache across worktrees" ON)

if(PA_USE_CCACHE AND NOT MSVC)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        if(CMAKE_C_COMPILER_LAUNCHER OR CMAKE_CXX_COMPILER_LAUNCHER)
            message(FATAL_ERROR
                "PA_USE_CCACHE=ON also sets up CMAKE_C_COMPILER_LAUNCHER and CMAKE_CXX_COMPILER_LAUNCHER.\n"
                "Either remove CMAKE_C_COMPILER_LAUNCHER and CMAKE_CXX_COMPILER_LAUNCHER or disable PA_USE_CCACHE."
            )
        endif()

        execute_process(
            COMMAND "${CCACHE_PROGRAM}" --version
            OUTPUT_VARIABLE _pa_ccache_version_output
            ERROR_QUIET
        )
        string(REGEX MATCH "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)" _ "${_pa_ccache_version_output}")
        set(_pa_ccache_version "${CMAKE_MATCH_1}")

        # ccache 4.8 is the first version that takes settings on the command line, which is how
        # PA_CCACHE_CONFIG reaches every compile.
        if(_pa_ccache_version VERSION_LESS 4.8)
            message(FATAL_ERROR
                "PA_USE_CCACHE=ON needs ccache 4.8 or newer, but ${CCACHE_PROGRAM} is version ${_pa_ccache_version}.\n"
                "Either upgrade ccache or disable PA_USE_CCACHE."
            )
        endif()

        execute_process(
            COMMAND "${CCACHE_PROGRAM}" --get-config cache_dir
            OUTPUT_VARIABLE _pa_ccache_main_dir
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(_pa_ccache_main_dir)
            set(_pa_ccache_default_config "cache_dir=${_pa_ccache_main_dir}/PA-Arduino-Source;base_dir=${_pa_checkout_dir}")
        else()
            set(_pa_ccache_default_config "base_dir=${_pa_checkout_dir}")
        endif()

        # set PA_CCACHE_CONFIG in CMake cache
        set(PA_CCACHE_CONFIG "${_pa_ccache_default_config}"
            CACHE STRING "ccache settings passed on every compile's command line, as key=value pairs")

        set(_pa_ccache_launcher "${CCACHE_PROGRAM}" ${PA_CCACHE_CONFIG})
        list(JOIN _pa_ccache_launcher " " _pa_ccache_launcher_text)
        message(STATUS "Using ccache: ${_pa_ccache_launcher_text}")
        set(CMAKE_C_COMPILER_LAUNCHER "${_pa_ccache_launcher}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${_pa_ccache_launcher}")

        if(PA_CCACHE_DEBUG_PATHS)
            # Only builds with debug info get these options.
            set(_pa_has_debug_info "$<CONFIG:Debug,RelWithDebInfo>")
            add_compile_options("$<${_pa_has_debug_info}:-fdebug-prefix-map=${_pa_checkout_dir}=.>")
            # Clang has this option, but not every compiler does. Without it, the prefix map
            # above still covers the compile folder when the build folder is inside the checkout.
            include(CheckCompilerFlag)
            check_compiler_flag(CXX "-fdebug-compilation-dir=." PA_HAS_DEBUG_COMPILATION_DIR)
            if(PA_HAS_DEBUG_COMPILATION_DIR)
                add_compile_options("$<${_pa_has_debug_info}:-fdebug-compilation-dir=.>")
            endif()
        endif()
    endif()
endif()
