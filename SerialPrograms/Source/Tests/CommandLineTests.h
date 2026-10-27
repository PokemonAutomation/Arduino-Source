/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Run all the unit tests from the command line, without launching the GUI.
 *
 *  How to run:
 *  Launch the program with the argument "--command-line-test-mode", or set
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "RUN" to true in SerialPrograms-Settings.json.
 *
 *  How the tests work:
 *  - Each kind of test is a class inheriting `UnitTest` (Common/Cpp/TestRunners/UnitTest.h)
 *    that implements `run()` and returns a `UnitTestResult` (passed, failed with a message,
 *    skipped or out of memory).
 *  - Each test case is an instance of such a class, constructed with its test file path and
 *    the expected results, e.g.
 *      database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/squirtle_Shiny.png", 7, 700052, 1);
 *    Test files are loaded relative to `UNIT_TEST_RESOURCE_PATH()`, which finds the
 *    CommandLineTests repo (https://github.com/PokemonAutomation/CommandLineTests) next to
 *    the program.
 *  - Each module registers its test cases in an `add_tests_<Name>(UnitTestDatabase&)` function in
 *    SerialPrograms/Source/ComputerPrograms/ComputerPrograms_UnitTestRunner.cpp.
 *  - The tests are run in parallel by `UnitTestRunner` (Common/Cpp/TestRunners/ParallelUnitTestRunner.h),
 *    within memory and thread limits.
 *
 *  Capture card distortions:
 *  Pass "--augment-color-distortions" to run image tests with color distortion augmentation.
 *  Each `ImageUnitTest` then also runs on its images re-colored like a few extreme capture card
 *  setups, and fails if any of those runs fails. Its failure message names each failing setting, e.g.
 *      [Win-MYPIN] Error: ...
 *  This needs the CaptureCardCalibration/ folder of the CommandLineTests repo: if it can't be
 *  loaded, no test runs and the program returns 1. See SerialPrograms/Source/Tests/ImageUnitTest.h
 *  and Common/Cpp/TestRunners/TestImageDistortionManager.h.
 *
 *  The same tests can also be run from the GUI with the "Unit Test Runner" program, which can
 *  run a single test, the tests matching a substring, or all tests.
 */


#ifndef PokemonAutomation_Tests_CommandLineTests_H
#define PokemonAutomation_Tests_CommandLineTests_H


namespace PokemonAutomation{


// Called by main() to run all the unit tests on command line, without launching any GUI.
// This function is only called when GlobalSettings::COMMAND_LINE_TEST_MODE is true.
// `augment_color_distortions` is set by the "--augment-color-distortions" argument: also run image tests
// with color distortion augmentation, using the distortion data from the test repo.
// Return 0 if all tests are passed, and 1 if any test failed or the distortion data can't be loaded.
int run_command_line_tests(bool augment_color_distortions);



}
#endif
