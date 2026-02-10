# This cmake file is to be included by CMakeLists.txt to build
# a GUI-free executable for Pokemon Automation utilities.

# Define sources for command-line executable
set(COMMANDLINE_SOURCES
    Source/CommandLine/CommandLine_Main.cpp
)

# Create command-line executable (GUI-free)
add_executable(SerialProgramsCommandLine ${COMMANDLINE_SOURCES})

# Link to SerialProgramsLib (contains all the core functionality)
target_link_libraries(SerialProgramsCommandLine PRIVATE SerialProgramsLib)

# Ensure SerialProgramsCommandLine rebuilds when SerialProgramsLib is updated
add_dependencies(SerialProgramsCommandLine SerialProgramsLib)

# Apply common target properties (includes, compile flags, etc.)
# This function is defined in the parent CMakeLists.txt
apply_common_target_properties(SerialProgramsCommandLine)
