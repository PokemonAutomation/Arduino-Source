# This cmake file is included by CMakeLists.txt when PA_PYTHON_BINDINGS is ON.
#
# It builds `_pa_core`, a pybind11 extension module that acts as a Nintendo Switch
# controller through a PABotBase2 serial device. It links only CoreLib (this
# codebase, GUI-free); it does not need Qt at runtime, OpenCV or Tesseract. Video
# capture and OCR are done by the pure-Python side of the `pokemon_automation`
# package with opencv-python / pytesseract.
#
# After each build the module is copied into Source/PythonBindings/pokemon_automation/
# so the package (and the MCP server in it) can be used straight from the source tree
# with `pip install -e Source/PythonBindings`.
#
# Configure with, for example:
#   cmake .. -DPA_PYTHON_BINDINGS=ON -DPython_EXECUTABLE=$(which python3)
# and build with:
#   cmake --build . --target _pa_core -j 10

# The extension module is a shared library, so everything linked into it must be
# position-independent.
set_target_properties(CoreLib PROPERTIES POSITION_INDEPENDENT_CODE ON)

# pybind11: use an installed copy if there is one, otherwise download it.
set(PYBIND11_FINDPYTHON ON)
find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG QUIET)
if (NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE PYBIND11_PIP_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (PYBIND11_PIP_CMAKE_DIR)
        find_package(pybind11 CONFIG QUIET PATHS "${PYBIND11_PIP_CMAKE_DIR}" NO_DEFAULT_PATH)
    endif()
endif()
if (NOT pybind11_FOUND)
    message(STATUS "pybind11 not found, downloading it")
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()
message(STATUS "Python bindings: building _pa_core for ${Python_EXECUTABLE} (${Python_VERSION})")

# The controller class it wraps (Source/Integrations/PybindSwitchController.*) is
# part of CoreLib, so only the module file is compiled here.
pybind11_add_module(_pa_core Source/PythonBindings/PythonBindings_Module.cpp)
pa_apply_gui_free_target_properties(_pa_core)
target_link_libraries(_pa_core PRIVATE CoreLib)

set(PA_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/PythonBindings/pokemon_automation)
add_custom_command(
    TARGET _pa_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_pa_core> ${PA_PYTHON_PACKAGE_DIR}/
    COMMENT "Copying _pa_core into the pokemon_automation Python package"
)
