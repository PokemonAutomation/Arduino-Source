# Set the minimum cmake version required
cmake_minimum_required(VERSION 3.18.0)

# Tell CMake to re-run configuration when these files change (get around Qt issues with stale CMake files)
set(_cmake_files_to_watch
    "${CMAKE_SOURCE_DIR}/CMakeLists.txt"
    "${CMAKE_SOURCE_DIR}/CMakePresets.json"
    "${CMAKE_SOURCE_DIR}/SourceFiles.cmake"
)

# Watch any helper .cmake files under cmake/
file(GLOB_RECURSE _cmake_helper_files "${CMAKE_SOURCE_DIR}/cmake/*.cmake")
list(APPEND _cmake_files_to_watch ${_cmake_helper_files})

# Append to the directory property so CMake will reconfigure when these change
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_cmake_files_to_watch})
#message(STATUS "CMake will reconfigure when these change: ${_cmake_files_to_watch}")

# Set the project name
project(SerialPrograms)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Produce clang tidy file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Set these three on globally so all intermediate Qt targets can get processed automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_custom_target(build-time-make-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory Assembly/)

# Determine if this is an internal repo and normalize REPO_ROOT_DIR
# CMAKE_CURRENT_SOURCE_DIR: the full path to this CmakeLists.txt
# Find if "Arduino-Source-Internal" is in the path and store the find result to internal_repo_position
string(FIND "${CMAKE_CURRENT_SOURCE_DIR}" "Arduino-Source-Internal" internal_repo_position)
if(internal_repo_position EQUAL -1)  # no "Arduino-Source-Internal" in the path
    # We are building the public repo, CMakeLists.txt is one level deep in the root repo dir
    set(REPO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
else()
    # We are building the internal repo, CMakeLists.txt is three levels deep in the root repo dir
    set(REPO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
endif()

# Determine environment
if(DEFINED ENV{GITHUB_ACTIONS})
    message(STATUS "Detected CI environment, skipping file deployment")
    set(DEPLOY_FILES FALSE)
else()
    message(STATUS "Detected local dev environment")
    if(PACKAGE_BUILD)
        set(DEST_DIR "$<TARGET_FILE_DIR:SerialPrograms>/Output")
        set(DEPLOY_DIR "${DEST_DIR}/Binaries64")
    else()
        set(DEST_DIR "$<TARGET_FILE_DIR:SerialPrograms>")
        set(DEPLOY_DIR "${DEST_DIR}")
    endif()

    set(DEPLOY_FILES TRUE)
endif()
#Find threads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Define the major version of Qt to search for
if(NOT QT_MAJOR)
    set(QT_MAJOR 6)
endif()

# Find Qt and windeployqt.exe relative to the Qt bin dir. If CI, not Windows, or not deploying, just find_package()
if(WIN32 AND DEPLOY_FILES)
    # Define the base Qt installation directory
    if(PREFERRED_QT_DIR AND PREFERRED_QT_VER)
        set(QT_CANDIDATE_DIR "${PREFERRED_QT_DIR}/${PREFERRED_QT_VER}/msvc2022_64/lib/cmake")
    endif()

    # Set default base directory just in case, validate that Qt specified in CMakePresets.json exists
    # Otherwise search out the latest available in the default Qt directory.
    set(QT_BASE_DIR "C:/Qt")
    if(QT_CANDIDATE_DIR AND EXISTS "${QT_CANDIDATE_DIR}")
        message(STATUS "Using preferred Qt directory for Qt${QT_MAJOR} ${PREFERRED_QT_VER}: ${QT_CANDIDATE_DIR}")
        set(CMAKE_PREFIX_PATH "${QT_CANDIDATE_DIR}")
        find_package(Qt${QT_MAJOR} ${PREFERRED_QT_VER} COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets REQUIRED)
    else()
        # Find all subdirectories in the Qt base directory
        find_package(Qt${QT_MAJOR} COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets REQUIRED)
        file(GLOB QT_SUBDIRS LIST_DIRECTORIES true "${QT_BASE_DIR}/${QT_MAJOR}*")

        # Filter and sort the directories to find the latest version
        list(SORT QT_SUBDIRS)
        list(REVERSE QT_SUBDIRS)

        # Get the first directory in the sorted list (latest version)
        list(GET QT_SUBDIRS 0 QT_LATEST_DIR)

        # Debug message to verify the latest Qt directory
        if(QT_LATEST_DIR)
            message(STATUS "Latest Qt directory for Qt${QT_MAJOR}: ${QT_LATEST_DIR}")
            set(CMAKE_PREFIX_PATH "${QT_LATEST_DIR}/msvc2022_64/lib/cmake")
        else()
            message(FATAL_ERROR "No Qt${QT_MAJOR} installation found in ${QT_BASE_DIR}")
        endif()
    endif()

    # Find windeployqt.exe for Qt dependency deployment
    get_target_property(qt_core_location Qt${QT_MAJOR}::Core LOCATION)
    get_filename_component(qt_bin_dir "${qt_core_location}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${qt_bin_dir}"
        PATH_SUFFIXES ../bin bin
        REQUIRED
    )
else()
    find_package(Qt${QT_MAJOR} COMPONENTS Widgets SerialPort Multimedia MultimediaWidgets REQUIRED)
endif()

# disable deprecated Qt APIs
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# add current directory to find tesseractPA.lib
link_directories(${CMAKE_CURRENT_LIST_DIR})

# add the list of all code file paths as ${LIBRARY_SOURCES} and ${EXECUTABLE_SOURCES}
include(SourceFiles.cmake)

# Organize source files in IDE project view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../ FILES ${LIBRARY_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../ FILES ${EXECUTABLE_SOURCES})

# Create library target first
add_library(SerialProgramsLib STATIC ${LIBRARY_SOURCES})

# Create binary target
if (APPLE)
    set(SerialPrograms_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../IconResource/icon.icns)
    # set_source_files_properties(SerialPrograms_ICON PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Links on how to create a MacOS app bundle with cmake
    # https://stackoverflow.com/questions/53560288/how-to-create-a-macos-app-bundle-with-cmake
    # https://cmake.org/cmake/help/latest/command/add_executable.html
    add_executable(SerialPrograms MACOSX_BUNDLE ${SerialPrograms_ICON} ${EXECUTABLE_SOURCES})
    set_target_properties(SerialPrograms PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER PokemonAutomation.SerialPrograms
        MACOSX_BUNDLE_BUNDLE_NAME SerialPrograms
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        # MacOSXBundleInfo.plist.in is modified from https://github.com/Kitware/CMake/blob/master/Modules/MacOSXBundleInfo.plist.in
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in
        RESOURCE ${SerialPrograms_ICON}
    )

    # make sure Packages repo, https://github.com/PokemonAutomation/Packages is placed in the same folder as this Arduino-Source repo
    # so the post-build command can copy the resources folder Packages/SerialPrograms/Resources/ into the built app bundle:
    set(RESOURCES_PATH "${REPO_ROOT_DIR}/../Packages/SerialPrograms/Resources")
    message(STATUS "Set Resources folder path from Packages repo: ${RESOURCES_PATH}")
    add_custom_command(
        TARGET SerialPrograms
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCES_PATH}"
        "$<TARGET_FILE_DIR:SerialPrograms>/../Resources"
    )
else()
    add_executable(SerialPrograms WIN32 ${EXECUTABLE_SOURCES})
endif()

target_link_libraries(SerialPrograms PRIVATE SerialProgramsLib)

# Add source code exclusive to the internal repo and add C++ macro of official release
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../Internal/SerialPrograms/Internal0.cpp")
    target_compile_definitions(SerialProgramsLib PRIVATE PA_OFFICIAL)
    target_compile_definitions(SerialPrograms PRIVATE PA_OFFICIAL)
    target_sources(SerialProgramsLib PRIVATE ../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.cpp)
    target_sources(SerialProgramsLib PRIVATE ../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h)
    target_sources(SerialProgramsLib PRIVATE ../../Internal/SerialPrograms/Internal0.cpp)
    target_sources(SerialProgramsLib PRIVATE ../../Internal/SerialPrograms/Internal1.cpp)
endif()

# Function to apply common properties to both library and executable targets
function(apply_common_target_properties target_name)
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(${target_name} PRIVATE Qt${QT_MAJOR}::Widgets Qt${QT_MAJOR}::SerialPort Qt${QT_MAJOR}::Multimedia Qt${QT_MAJOR}::MultimediaWidgets)
    target_link_libraries(${target_name} PRIVATE Threads::Threads)

    #add defines
    target_compile_definitions(${target_name} PRIVATE NOMINMAX DPP_NO_DEPRECATED)

    #add include directory
    target_include_directories(${target_name} SYSTEM PRIVATE ../3rdParty/)
    target_include_directories(${target_name} PRIVATE ../ ../../Internal/ Source/)
    target_link_directories(${target_name} PRIVATE ../3rdPartyBinaries/)
endfunction()

# Apply common properties to both targets
apply_common_target_properties(SerialProgramsLib)
apply_common_target_properties(SerialPrograms)

if (WIN32)
    add_library(OpenCV_lib IMPORTED UNKNOWN)
    target_include_directories(SerialProgramsLib SYSTEM PRIVATE ../3rdParty/opencv-4.11.0/)
    set_target_properties(OpenCV_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/opencv_world4110.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/opencv_world4110d.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    add_library(ONNX_lib IMPORTED UNKNOWN)
    target_include_directories(SerialProgramsLib SYSTEM PRIVATE ../3rdParty/ONNX/)
    set_target_properties(ONNX_lib PROPERTIES
        IMPORTED_LOCATION   ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/onnxruntime.lib
    )

    add_library(ONNX_Providers_lib IMPORTED UNKNOWN)
    set_target_properties(ONNX_Providers_lib PROPERTIES
        IMPORTED_LOCATION   ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/onnxruntime_providers_shared.lib
    )

    add_library(dpp_lib IMPORTED UNKNOWN)
    set_target_properties(dpp_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/dpp.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/dppd.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    add_library(discord_lib IMPORTED UNKNOWN)
    target_include_directories(SerialProgramsLib SYSTEM PRIVATE ../3rdParty/discord_social_sdk_win/)
    set_target_properties(discord_lib PROPERTIES
        IMPORTED_LOCATION_RELEASE           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_win/lib/release/discord_partner_sdk.lib
        IMPORTED_LOCATION_DEBUG             ${CMAKE_CURRENT_SOURCE_DIR}/../3rdPartyBinaries/discord_social_sdk_win/lib/debug/discord_partner_sdk.lib
        MAP_IMPORTED_CONFIG_DEBUG           DEBUG
        MAP_IMPORTED_CONFIG_RELEASE         RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO  RELEASE
        MAP_IMPORTED_CONFIG_MINSIZEREL      RELEASE
    )

    target_link_libraries(
        SerialProgramsLib PRIVATE
        tesseractPA.lib
        OpenCV_lib
        ONNX_lib
        ONNX_Providers_lib
        dpp_lib
        discord_lib
    )

    target_compile_definitions(
        SerialProgramsLib PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
        PA_TESSERACT
        PA_DPP
        PA_SOCIAL_SDK
    )

    target_compile_options(SerialProgramsLib PRIVATE /FAs /FaAssembly/ /MP /W4 /WX /external:anglebrackets /external:W0 /utf-8)
    target_compile_options(SerialProgramsLib PRIVATE /wd5054)  # Deprecated enum arithemtic
    target_compile_options(SerialProgramsLib PRIVATE /wd4505)  # unreferenced local function has been removed

    set(ARCH_FLAGS_09_Nehalem       /W4)    # Dummy parameter
    set(ARCH_FLAGS_13_Haswell       /arch:AVX2)
    set(ARCH_FLAGS_17_Skylake       /arch:AVX512)
    set(ARCH_FLAGS_19_IceLake       /arch:AVX512)

    if(CMAKE_GENERATOR_TOOLSET MATCHES "ClangCL")
        target_compile_options(SerialProgramsLib PRIVATE -Wno-unused-function)
        target_compile_options(SerialProgramsLib PRIVATE -march=nehalem)
        set(ARCH_FLAGS_09_Nehalem       -march=nehalem)
        set(ARCH_FLAGS_13_Haswell       -march=haswell)
        set(ARCH_FLAGS_17_Skylake       -march=skylake-avx512)
        set(ARCH_FLAGS_19_IceLake       -march=icelake-client)
    endif()

    #   Run-time ISA dispatching
    target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_08_Nehalem)
    target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_13_Haswell)
    target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_17_Skylake)
    target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_19_IceLake)

#    target_compile_options(SerialProgramsLib PRIVATE /arch:AVX2   /DPA_Arch_x64_AVX2)
#    target_compile_options(SerialProgramsLib PRIVATE /arch:AVX512 /DPA_Arch_x64_AVX512)
#    target_compile_options(SerialProgramsLib PRIVATE /arch:AVX512 /DPA_Arch_x64_AVX512GF)
else() # macOS and Linux
    if (APPLE)
        # Get root path where Homebrew installs libraries. Example root path: "/opt/homebrew/",
        # where you can find libraries in "/opt/homebrew/lib/" and headers in "/opt/homebrew/include/".
        # CMake does not support ONNX Runtime, so we have to add ONNX Runtime paths on our own.
        # Save the root path as CMake variable HOMEBREW_PREFIX.
        execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

        # ==== Get ONNX Runtime paths ====
        # First, try to see if there is an ONNX Runtime library built from source.
        # Assume the onnxruntime repo, https://github.com/microsoft/onnxruntime is placed in the same folder as
        # this Arduino-Source repo:
        set(ONNXRUNTIME_ROOTDIR "${REPO_ROOT_DIR}../onnxruntime/")
        set(ONNXRUNTIME_HEADER_DIR "${ONNXRUNTIME_ROOTDIR}include/onnxruntime/core/session/")
        if(EXISTS "${ONNXRUNTIME_HEADER_DIR}")
            # we have ONNX Runtime built from source
            message("Use ONNX Runtime built from source at ${ONNXRUNTIME_ROOTDIR}")
            target_include_directories(SerialProgramsLib PRIVATE ${ONNXRUNTIME_HEADER_DIR})
            target_link_libraries(SerialProgramsLib PRIVATE ${ONNXRUNTIME_ROOTDIR}/build/MacOS/Release/libonnxruntime.dylib)
        else()
            message("Built-from-source ONNX Runtime folder ${ONNXRUNTIME_ROOTDIR} does not exist.")
            message("Use ONNX Runtime installed by Homebrew.")
            target_include_directories(SerialProgramsLib PRIVATE ${HOMEBREW_PREFIX}/include/onnxruntime)
            target_link_libraries(SerialProgramsLib PRIVATE ${HOMEBREW_PREFIX}/lib/libonnxruntime.dylib)
        endif()
        # Find OpenCV
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64)|(ARM64)")
            find_package(OpenCV REQUIRED HINTS "/opt/homebrew/opt/opencv/lib/cmake/opencv4/")
        else()
            find_package(OpenCV REQUIRED HINTS "/usr/local/opt/opencv/lib/cmake/opencv4/")
        endif()

    else() # Linux
        # ONNX RUNTIME LINUX CONFIG
    # NOTE: users can specify their own ONNX_ROOT_PATH (this is the base folder for ONNX) on the command line when evoking cmake.
        # this can be done via -DONNX_ROOT_PATH=/path/to/onnx

        # If the path isn't provided, it'll automatically download the 1.23.0 release to the build folder,
        # extract it, and then link it up

        # Set the ONNXRUNTIME version and arch, allows for quick updates
        set(ONNXRUNTIME_VERSION "1.23.0")
        set(ONNXRUNTIME_ARCH "x64")
        set(ONNXRUNTIME_DIR_NAME "onnxruntime-linux-${ONNXRUNTIME_ARCH}-${ONNXRUNTIME_VERSION}")
        set(ONNXRUNTIME_TGZ_NAME "${ONNXRUNTIME_DIR_NAME}.tgz")
        # build up the URL based on the version and name
        set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/${ONNXRUNTIME_TGZ_NAME}")

        # Cache variable is editable by users *and* is properly stored
        set(ONNX_ROOT_PATH "" CACHE PATH "Path to the root of a pre-installed ONNX Runtime (e.g., /path/to/onnxruntime-linux-x64-1.23.0)")

        # Download the onnxruntime if it doesn't exist or if the path is bad
        if(NOT ONNX_ROOT_PATH OR NOT EXISTS "${ONNX_ROOT_PATH}/include/onnxruntime_cxx_api.h")
            set(EXTRACTED_ONNX_PATH "${CMAKE_BINARY_DIR}/${ONNXRUNTIME_DIR_NAME}")

            if(NOT IS_DIRECTORY ${EXTRACTED_ONNX_PATH})
                message(STATUS "ONNX_ROOT_PATH not found. Downloading ONNX Runtime v${ONNXRUNTIME_VERSION}...")
                
                # logic for downloading...
                set(DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/${ONNXRUNTIME_TGZ_NAME}")
                file(DOWNLOAD
                    ${ONNXRUNTIME_URL}
                    ${DOWNLOAD_LOCATION}
                    SHOW_PROGRESS
                    STATUS download_status
                )
                list(GET download_status 0 error_code)
                if(NOT error_code EQUAL 0)
                    list(GET download_status 1 error_message)
                    message(FATAL_ERROR "Failed to download ONNX Runtime: ${error_message}")
                endif()

                # logic for extracting the tarball to our working directory
                message(STATUS "Extracting ONNX Runtime...")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_LOCATION}"
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                )
            endif()

            # Update the ONNX_ROOT_PATH to point to our downloaded version, force ensures we update it
            set(ONNX_ROOT_PATH "${EXTRACTED_ONNX_PATH}" CACHE PATH "Path to the downloaded ONNX Runtime" FORCE)
            message(STATUS "Using downloaded ONNX Runtime at: ${ONNX_ROOT_PATH}")
        else()
            message(STATUS "Using user-provided ONNX Runtime at: ${ONNX_ROOT_PATH}")
        endif()

        # configure and find headers from the onnxruntime
        find_path(ONNX_INCLUDE_DIRS
            NAMES onnxruntime_cxx_api.h
            HINTS "${ONNX_ROOT_PATH}/include"
            REQUIRED
        )

        find_library(ONNX_RUNTIME_LIB
            NAMES onnxruntime
            HINTS "${ONNX_ROOT_PATH}/lib"
            REQUIRED
        )

        if(ONNX_INCLUDE_DIRS AND ONNX_RUNTIME_LIB)
            # only link if it was found, otherwise we error out
            message(STATUS "Found ONNX Include Dirs: ${ONNX_INCLUDE_DIRS}")
            message(STATUS "Found ONNX Library: ${ONNX_RUNTIME_LIB}")
            target_include_directories(SerialProgramsLib PUBLIC "${ONNX_INCLUDE_DIRS}")
            target_link_libraries(SerialProgramsLib PUBLIC "${ONNX_RUNTIME_LIB}")
        else()
            message(FATAL_ERROR "Could not find ONNX Runtime headers or library.")
        endif()
        # Find OpenCV
        find_package(OpenCV REQUIRED HINTS "/usr/local/opt/opencv/lib/cmake/opencv4/")
    endif()

    include_directories(${OpenCV_INCLUDE_DIRS})
    link_directories(${OpenCV_LIBRARY_DIRS})
    target_link_libraries(SerialProgramsLib PRIVATE ${OpenCV_LIBS})

    #we hope to use our own Tesseract build in future so we can rid that dependency
    #but right now to run on Linux and Mac we need to use external Tesseract library
    if (UNIX_LINK_TESSERACT)
        find_package(PkgConfig REQUIRED)
        target_compile_definitions(SerialProgramsLib PRIVATE PA_TESSERACT UNIX_LINK_TESSERACT)
        pkg_search_module(TESSERACT REQUIRED tesseract)
        pkg_search_module(LEPTONICA REQUIRED lept)
        include_directories(${TESSERACT_INCLUDE_DIRS})
        include_directories(${LEPTONICA_INCLUDE_DIRS})
        link_directories(${TESSERACT_LIBRARY_DIRS})
        link_directories(${LEPTONICA_LIBRARY_DIRS})
        target_link_libraries(SerialProgramsLib PRIVATE ${TESSERACT_LINK_LIBRARIES})
        target_link_libraries(SerialProgramsLib PRIVATE ${LEPTONICA_LINK_LIBRARIES})
    endif()

    # enable dpp integration
    if ((APPLE) AND (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64)|(ARM64)"))
        add_library(libdpp STATIC IMPORTED)
        set_target_properties(libdpp PROPERTIES
            IMPORTED_LOCATION ${REPO_ROOT_DIR}/3rdPartyBinaries/libdpp_macos_arm64.a
            INTERFACE_COMPILE_DEFINITIONS "PA_DPP"
            INTERFACE_LINK_LIBRARIES "-L/opt/homebrew/lib -lssl -lcrypto -lopus -lsodium -lz" # add dpp's deps
        )
        target_link_libraries(SerialProgramsLib PRIVATE libdpp)
    endif()

    if (APPLE)
        # Add -Wno-c11-extensions to avoid clang gives
        # /usr/local/Cellar/opencv/4.5.5_3/include/opencv4/opencv2/core/mat.inl.hpp:2116:9: error: '_Atomic' is a C11 extension
        # when compiling OpenCV
        # target_compile_options(SerialProgramsLib PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-c11-extensions)
        target_compile_options(SerialProgramsLib PRIVATE -Wall -Wextra -Wpedantic -Werror -Wshorten-64-to-32)
    else()
        # Assume GCC
        target_compile_options(SerialProgramsLib PRIVATE -Wall -Wextra -Wpedantic -Werror -fno-strict-aliasing)
    endif()

    # Set OS-specific flags
    if (APPLE)
        # on macOS, need this framework to query OS API to control display sleep and system sleep behavior
        target_link_libraries(SerialProgramsLib PRIVATE "-framework IOKit -framework CoreFoundation")
    endif()

    IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        # Arm CPU
        #   Run-time ISA dispatching
        target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_arm64_20_M1)

    else()
        # Intel CPU
        target_compile_options(SerialProgramsLib PRIVATE -march=nehalem)
        set(ARCH_FLAGS_09_Nehalem       -march=nehalem)
        set(ARCH_FLAGS_13_Haswell       -march=haswell)
        set(ARCH_FLAGS_17_Skylake       -march=skylake-avx512)
        set(ARCH_FLAGS_19_IceLake       -march=icelake-client)

        #   Run-time ISA dispatching
        target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_08_Nehalem)
        target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_13_Haswell)
        target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_17_Skylake)
        target_compile_definitions(SerialProgramsLib PRIVATE PA_AutoDispatch_x64_19_IceLake)

    endif()
endif()



#   Run-time CPU dispatching.

if (ARCH_FLAGS_09_Nehalem)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/AudioStreamConversion/AudioStreamConversion_Core_x86_SSE41.cpp
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_SSE41.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_SSE42.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_SSE42.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_SSE41.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_SSE41.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_SSE.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_SSE41.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x8_x64_SSE42.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x8_x64_SSE42.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_09_Nehalem}
)
endif()
if (ARCH_FLAGS_13_Haswell)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/AbsFFT/Kernels_AbsFFT_Core_x86_AVX2.cpp
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX2.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX2.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX2.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX2.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX2.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX2.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x16_x64_AVX2.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x16_x64_AVX2.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x16_x64_AVX2.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_13_Haswell}
)
endif()
if (ARCH_FLAGS_17_Skylake)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/ImageFilters/Kernels_ImageFilter_Basic_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range_x64_AVX512.cpp
    Source/Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean_x64_AVX512.cpp
    Source/Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqr_x64_AVX512.cpp
    Source/Kernels/ImageStats/Kernels_ImagePixelSumSqrDev_x64_AVX512.cpp
    Source/Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch_Core_x86_AVX512.cpp
    Source/Kernels/SpikeConvolution/Kernels_SpikeConvolution_Core_x86_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryMatrix/Kernels_BinaryMatrix_Core_64x64_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x32_x64_AVX512.cpp
    Source/Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_64x64_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_17_Skylake}
)
endif()
if (ARCH_FLAGS_19_IceLake)
SET_SOURCE_FILES_PROPERTIES(
    Source/Kernels/ImageFilters/RGB32_Brightness/Kernels_ImageFilter_RGB32_Brightness_x64_AVX512-VNNI.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.cpp
    Source/Kernels/Waterfill/Kernels_Waterfill_Core_64x64_x64_AVX512-GF.cpp
    PROPERTIES COMPILE_FLAGS ${ARCH_FLAGS_19_IceLake}
)
endif()

if (WIN32 AND NOT DEPLOY_FILES)
    file(GLOB MY_DLLS
        "../3rdPartyBinaries/*.dll"
    )
    file(COPY ${MY_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    message(STATUS "Extracting opencv_world4110d")
    file(ARCHIVE_EXTRACT
      INPUT ${REPO_ROOT_DIR}/3rdPartyBinaries/opencv_world4110d.zip
      DESTINATION ${REPO_ROOT_DIR}/3rdPartyBinaries/
    )

    set(DISCORD_SDK_ZIP "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win.zip")
    set(DISCORD_SDK_DIR "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win")
    if(EXISTS "${DISCORD_SDK_ZIP}" AND NOT EXISTS "${DISCORD_SDK_DIR}")
        message(STATUS "Extracting Social SDK")
        file(ARCHIVE_EXTRACT
            INPUT "${DISCORD_SDK_ZIP}"
            DESTINATION "${REPO_ROOT_DIR}/3rdPartyBinaries"
        )
    endif()
endif()

if (WIN32 AND DEPLOY_FILES)
    set(PACKAGES_DIR "${REPO_ROOT_DIR}/Packages")
    set(OPENCV_DEBUG_ZIP "${REPO_ROOT_DIR}/3rdPartyBinaries/opencv_world4110d.zip")
    set(OPENCV_DEBUG_DLL "${REPO_ROOT_DIR}/3rdPartyBinaries/opencv_world4110d.dll")

    set(DISCORD_ZIP "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win.zip")
    set(DISCORD_DIR "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win")
    set(DISCORD_DLL_DEBUG "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win/bin/debug/discord_partner_sdk.dll")
    set(DISCORD_DLL_RELEASE "${REPO_ROOT_DIR}/3rdPartyBinaries/discord_social_sdk_win/bin/release/discord_partner_sdk.dll")

    set(FIRMWARE_DIR "${PACKAGES_DIR}/PABotBase/PABotBase-Switch")
    set(SCRIPTS_DIR "${PACKAGES_DIR}/SerialPrograms/Scripts")
    set(RESOURCES_DIR "${PACKAGES_DIR}/SerialPrograms/Resources")

    # Clone Packages repo if not already present
    if (NOT EXISTS "${PACKAGES_DIR}")
        message(STATUS "Cloning Packages repo at configure time...")
        execute_process(
            COMMAND git clone --branch master https://github.com/PokemonAutomation/Packages "${PACKAGES_DIR}"
            WORKING_DIRECTORY "${REPO_ROOT_DIR}"
        )
        add_custom_target(clone_packages DEPENDS "${PACKAGES_DIR}")
        add_dependencies(SerialPrograms clone_packages)
    else()
        add_custom_target(clone_packages COMMENT "Packages repo already present, skipping clone.")
    endif()

    # Copy resources if not already present
    if(NOT EXISTS "${DEST_DIR}/Resources")
        add_custom_command(
            TARGET SerialPrograms POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Copying resources"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_DIR}" "${DEST_DIR}/Resources"
        )
    endif()

    # Copy firmware if not already present
    if(NOT EXISTS "${DEST_DIR}/Firmware")
        add_custom_command(
            TARGET SerialPrograms POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Copying firmware"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${FIRMWARE_DIR}" "${DEST_DIR}/Firmware"
        )
    endif()

    # Copy scripts if not already present
    if(NOT EXISTS "${DEST_DIR}/Scripts")
        add_custom_command(
            TARGET SerialPrograms POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Copying scripts"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${SCRIPTS_DIR}" "${DEST_DIR}/Scripts"
        )
    endif()

    # Run windeployqt if Qt DLLs are missing
    if(WINDEPLOYQT_EXECUTABLE AND NOT EXISTS "${DEPLOY_DIR}/Qt6Core.dll")
        add_custom_command(
            TARGET SerialPrograms POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Running windeployqt"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --dir "${DEPLOY_DIR}"
                $<IF:$<CONFIG:Debug>,--debug,--release>
                $<TARGET_FILE:SerialPrograms>
        )
        add_custom_target(run_windeployqt DEPENDS $<TARGET_FILE:SerialPrograms>)
    else()
        message(WARNING "windeployqt not found, skipping Qt deployment.")
    endif()

    # Copy Discord SDK DLL
    add_custom_command(
        TARGET SerialPrograms POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Copying Discord SDK DLL"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${DISCORD_DLL_DEBUG},${DISCORD_DLL_RELEASE}>"
            "${DEPLOY_DIR}/discord_partner_sdk.dll"
    )

    # Copy 3rd party DLLs if missing/different
    file(GLOB THIRD_PARTY_DLLS "${REPO_ROOT_DIR}/3rdPartyBinaries/*.dll")
    foreach(DLL IN LISTS THIRD_PARTY_DLLS)
        get_filename_component(DLL_NAME "${DLL}" NAME)
        add_custom_command(
            TARGET SerialPrograms POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${DEPLOY_DIR}/${DLL_NAME}"
        )
    endforeach()

    # Move built .exe and .pdb to deploy folder, print if pdb is missing
    add_custom_command(
        TARGET SerialPrograms POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Moving built exe and pdb"
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:SerialPrograms> "${DEPLOY_DIR}/$<TARGET_FILE_NAME:SerialPrograms>"
        COMMAND cmd /c "if exist \"$<TARGET_PDB_FILE:SerialPrograms>\" ( \"${CMAKE_COMMAND}\" -E rename \"$<TARGET_PDB_FILE:SerialPrograms>\" \"${DEPLOY_DIR}/$<TARGET_PDB_FILE_NAME:SerialPrograms>\" ) else ( echo [Deploy] PDB file missing: $<TARGET_PDB_FILE:SerialPrograms> )"
    )

    # Extract Discord SDK if missing
    if (NOT EXISTS "${DISCORD_DIR}")
        message(STATUS "Discord SDK not found, extracting from zip...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${DISCORD_ZIP}"
            WORKING_DIRECTORY "${REPO_ROOT_DIR}/3rdPartyBinaries"
        )
    endif()

    # Extract OpenCV debug DLL if missing
    if (NOT EXISTS "${OPENCV_DEBUG_DLL}")
        message(STATUS "OpenCV debug DLL not found, extracting from zip...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${OPENCV_DEBUG_ZIP}"
            WORKING_DIRECTORY "${REPO_ROOT_DIR}/3rdPartyBinaries"
        )
    endif()

    # Final target that depends on all deployment steps
    add_custom_target(DeployRuntime ALL
        DEPENDS SerialPrograms clone_packages run_windeployqt
        COMMENT "Deploying runtime (Qt, packages, DLLs) to ${DEPLOY_DIR}"
        VERBATIM
    )
endif()

if (QT_MAJOR GREATER_EQUAL 6)
    qt_finalize_target(SerialPrograms)
endif()
