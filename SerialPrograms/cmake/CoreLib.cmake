# CoreLib: the GUI-free core of SerialPrograms.
#
# CoreLib contains the serial connection, the PABotBase2 protocol, controller
# scheduling and the Nintendo Switch controllers, plus the utilities they need
# (logging, threads, options, etc.). It is built from ${CORE_LIBRARY_SOURCES}
# (see SourceFiles.cmake) and needs no third-party libraries: no Qt, OpenCV,
# ONNX Runtime, Tesseract or DPP. Qt-specific code in these files is guarded by
# `#ifdef QT_CORE_LIB`, which is never defined for CoreLib.
#
# Users of CoreLib:
#   - SerialProgramsCommandLine (Source/CommandLine/)
#   - the `_pa_core` Python module (Source/PythonBindings/)
# The GUI program does not link CoreLib; SerialProgramsLib compiles the same
# sources itself with Qt enabled.
#
# This file is included by CMakeLists.txt for both the full build and the core-only
# build (-DPA_CORE_ONLY=ON). It requires ${CORE_LIBRARY_SOURCES} and Threads.
#
# It also defines cmake function:
# `pa_apply_base_target_properties()`: sets up base properties (include directores
# and definitions) for every target (GUI or not) in this project.
# `pa_apply_gui_free_target_properties()`: sets up properties for GUI-free targets.
#   It calls `pa_apply_base_target_properties()` and sets properties to remove GUI
#   dependencies and code.

#  The SerialPrograms folder, normalized so include paths are spelled exactly as
#  before this file existed (a different spelling would force a full rebuild).
get_filename_component(PA_SERIAL_PROGRAMS_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)


# Include directories, definitions and threading shared by all targets.
function(pa_apply_base_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(${target_name} PRIVATE Threads::Threads)

    #add defines
    target_compile_definitions(${target_name} PRIVATE NOMINMAX DPP_NO_DEPRECATED)

    #add include directory
    target_include_directories(${target_name} SYSTEM PRIVATE ${PA_SERIAL_PROGRAMS_DIR}/../3rdParty-Core)
    target_include_directories(${target_name} PRIVATE
        ${PA_SERIAL_PROGRAMS_DIR}/..
        ${PA_SERIAL_PROGRAMS_DIR}/../../Internal/
        ${PA_SERIAL_PROGRAMS_DIR}/Source/
    )
endfunction()

# Base properties for a target that must not depend on Qt or any other GUI library.
# Qt-only code paths are compiled out, and images use no GUI backend.
function(pa_apply_gui_free_target_properties target_name)
    pa_apply_base_target_properties(${target_name})
    target_compile_definitions(${target_name} PRIVATE PA_IMAGE_BACKEND_None)
    #  The global CMAKE_AUTOMOC etc. would make CMake look for Qt's code generators.
    set_target_properties(${target_name} PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
endfunction()


# Create GUI-free Controller library
add_library(CoreLib STATIC ${CORE_LIBRARY_SOURCES})
pa_apply_gui_free_target_properties(CoreLib)

if (WIN32)
    #  /FaAssembly/ writes assembly listings to Assembly/ in the build folder.
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Assembly)
    target_compile_options(CoreLib PRIVATE /FAs /FaAssembly/ /MP /W4 /WX /external:anglebrackets /external:W0 /utf-8)
    target_compile_options(CoreLib PRIVATE /wd5054)  # Deprecated enum arithemtic
    target_compile_options(CoreLib PRIVATE /wd4505)  # unreferenced local function has been removed

    if(CMAKE_GENERATOR_TOOLSET MATCHES "ClangCL")
        target_compile_options(CoreLib PRIVATE -Wno-unused-function)
        target_compile_options(CoreLib PRIVATE -march=nehalem)
    endif()
else() # macOS and Linux
    if(APPLE)
        target_compile_options(CoreLib PRIVATE -Wall -Wextra -Wpedantic -Werror -Wshorten-64-to-32)
        target_link_libraries(CoreLib PRIVATE "-framework IOKit -framework CoreFoundation")
    else() # Linux
        # Assume GCC
        target_compile_options(CoreLib PRIVATE -Wall -Wextra -Wpedantic -Werror -fno-strict-aliasing)
    endif()

    if(NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "^(arm|arm64|aarch64)")
        # Intel CPU
        target_compile_options(CoreLib PRIVATE -march=nehalem)
    endif()
endif()
