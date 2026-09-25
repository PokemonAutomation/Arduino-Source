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
 *  - Each module registers its test cases in an `add_tests(UnitTestDatabase&)` function, and
 *    ComputerPrograms/UnitTestRunner.cpp collects all of them.
 *  - The tests are run in parallel by `ParallelUnitTestRunner`, within memory and thread limits.
 *
 *  The same tests can also be run from the GUI with the "Unit Test Runner" program, which can
 *  run a single test, the tests matching a substring, or all tests.
 */


#ifndef PokemonAutomation_Tests_CommandLineTests_H
#define PokemonAutomation_Tests_CommandLineTests_H


namespace PokemonAutomation{


// Called by main() to run all the unit tests on command line, without launching any GUI.
// This function is only called when GlobalSettings::COMMAND_LINE_TEST_MODE is true.
// Return 0 if all tests are passed.
int run_command_line_tests();



}
#endif
