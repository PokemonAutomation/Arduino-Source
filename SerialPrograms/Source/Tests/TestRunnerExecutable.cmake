# This cmake file is to be included by CMakeLists.txt to build SerialProgramsTests, an executable
# that runs the unit tests without the GUI (see Source/Tests/TestRunner_Main.cpp).

add_executable(SerialProgramsTests Source/Tests/TestRunner_Main.cpp)

# The tests are registered by explicit calls, not by static constructors, so the linker may
# leave out the parts of the library that the tests don't use.
target_link_libraries(SerialProgramsTests PRIVATE SerialProgramsLib)
apply_common_target_properties(SerialProgramsTests)

# The tests read Resources/ from in or above the executable's folder, like SerialPrograms.
add_dependencies(SerialProgramsTests clone_packages)
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/Resources")
    add_custom_command(
        TARGET SerialProgramsTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Deploy] Copying resources ${RESOURCES_DIR} to $<TARGET_FILE_DIR:SerialProgramsTests>/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCES_DIR}" "$<TARGET_FILE_DIR:SerialProgramsTests>/Resources"
    )
endif()
