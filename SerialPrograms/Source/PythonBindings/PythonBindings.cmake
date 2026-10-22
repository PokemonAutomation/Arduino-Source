# Python bindings: the `_pa_core` module.
#
# This file is included by CMakeLists.txt when PA_PYTHON_BINDINGS is ON, for both
# the full build and the core-only build (-DPA_CORE_ONLY=ON).
#
# It builds `_pa_core`, a pybind11 extension module that lets Python act as a
# Nintendo Switch controller through a PABotBase2 serial device. It links only
# CoreLib (this codebase, GUI-free); it needs no Qt, OpenCV, ONNX Runtime or
# Tesseract. See Source/PythonBindings/README.md for usage.
#
# Configure with, for example:
#   cmake .. -DPA_PYTHON_BINDINGS=ON [-DPython_EXECUTABLE=$(which python3)]
# and build with:
#   cmake --build . --target _pa_core
#
# After each build the module is copied into the `pokemon_automation` package
# (Source/PythonBindings/pokemon_automation/), so the package can be used
# straight from the source tree (or with `pip install -e Source/PythonBindings`).

# The extension module is a shared library, so everything linked into it must be
# position-independent.
set_target_properties(CoreLib PROPERTIES POSITION_INDEPENDENT_CODE ON)

# pybind11: use an installed copy (a system package or `pip install pybind11`).
# It is deliberately NOT downloaded here: configure fails with instructions
# instead, so builds never reach out to the network on their own.
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
    message(FATAL_ERROR
        "Python bindings: pybind11 was not found for ${Python_EXECUTABLE} (${Python_VERSION}).\n"
        "This build does not download pybind11 automatically. Install it and reconfigure, e.g.:\n"
        "    \"${Python_EXECUTABLE}\" -m pip install pybind11\n"
        "or use a system package (Debian/Ubuntu: `sudo apt install pybind11-dev`, macOS: `brew install pybind11`)."
    )
endif()
message(STATUS "Python bindings: building _pa_core for ${Python_EXECUTABLE} (${Python_VERSION})")

# The controller class the module wraps (Source/Integrations/PybindSwitchController.*)
# is part of CoreLib, so only the module file is compiled here.
pybind11_add_module(_pa_core Source/PythonBindings/PythonBindings_Module.cpp)
pa_apply_gui_free_target_properties(_pa_core)
target_link_libraries(_pa_core PRIVATE CoreLib)

# Copy the built module into the package next to the Python sources.
add_custom_command(
    TARGET _pa_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_pa_core>
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/PythonBindings/pokemon_automation/
    COMMENT "Copying _pa_core into the pokemon_automation Python package"
)
