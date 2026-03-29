#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDBusCpp::sdbus-c++" for configuration "Release"
set_property(TARGET SDBusCpp::sdbus-c++ APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SDBusCpp::sdbus-c++ PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C;CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsdbus-c++.a"
  )

list(APPEND _cmake_import_check_targets SDBusCpp::sdbus-c++ )
list(APPEND _cmake_import_check_files_for_SDBusCpp::sdbus-c++ "${_IMPORT_PREFIX}/lib/libsdbus-c++.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
