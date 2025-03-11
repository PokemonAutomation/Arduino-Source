/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Explanation of the command line test framework
 * 
 *  Goals:
 * 
 *  - To build a unit test-like framework to ensure correctness of the various inference methods.
 *  - To allow efficient iterations during development of a new inference method without the need to launch GUI and click buttons.
 * 
 * 
 *  How to use the test folder:
 * 
 *  Enable the command line test mode by changing the SerialPrograms-Settings.json field:
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "RUN" to true.
 *  In this mode, the program does not launch GUI. Instead it looks for a local folder which path is specified by the json field:
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "FOLDER"
 *  and run tests inside it.
 *  The folder is structured as:
 *  ../CommandLineTests/                         <- root test folder
 *    - PokemonLA/                               <- test space, sub-folder to organize tests, e.g. by which games the inferences are used in
 *        - BattleMenuDetector/                    <- test object, name of the class/function/file to test
 *            - IngoBattleDayTime_True.png           <- test file for BattleMenuDetector
 *            - IngoBattleNightTime_True.png         <- test file for BattleMenuDetector
 *    - PokemonBDSP/                             <- another test space for inferences in another game
 *        - DialogDetector/                        <- test object, this time it's DialogDetector for BDSP
 *            - Win_Mirabox/                         <- can have more folders under test object to organize test files, e.g by OS and capture card
 *                - FetchEggDayTime_True.png           <- test file for DialogDetector
 * 
 *  The test framework will go to each test object's folder and use its path to determine which test object to call.
 *  For example, if the path is CommandLineTest/PokemonLA/BattleMenuDetector/, the program will test the code in
 *  Source/PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h.
 * 
 *  You can also selectively run some of the tests by setting
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "TEST_LIST" as a list of strings.
 *  Each string will be a relative path. It can be any of:
 *  - "PokemonBDSP"
 *  - "PokemonBDSP/DialogDetector"
 *  - "PokemonBDSP/DialogDetector/Win_Mirabox"
 *  - "PokemonBDSP/DialogDetector/Win_Mirabox/FetchEggDayTime_True.png"
 *  This gives the flexibility to test the code for a game, a detector, a detector on a capture card or a detector on a particular image/audio/video.
 * 
 *  If you have put some test files for experimental code in a folder and later decide to not run that code for a while, you can use
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "IGNORE_LIST" as a list of strings to skip the paths to those tests.
 *  Each string in the list serves as a prefix to the test path that the test framework uses to filter out paths.
 *  
 * Those "hidden" files are useful for storing some metadata in the folder, or serving as an extra file in case some tests need more than one test files.
 * 
 *  How to add new test code:
 * 
 *  The test framework calls TestMap.h: find_test_function(test_space, test_obj_name) to find the test function related to a test path.
 *  For example, given a path PokemonLA/BattleMenuDetector/IngoBattleDayTime_True.png, by extracting test_space as "PokemonLA" and test_obj_name as
 *  "BattleMenuDetector" from the path, find_test_function() returns the code that runs
 *  Source/PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h:BattleMenuDetector.
 *  
 *  Internally, find_test_function() searches in a map: TestMap.cpp:std::map<std::string, TestFunction> TEST_MAP to get the correct test function.
 *  The key of the map in the above example is "PokemonLA_BattleMenuDetector" (we use "_" to connect test space and test obj name). The value of the
 *  map is TestFunction = std::function<int(const std::string& test_file_path)>.
 *  Each implemented TestFunction needs to:
 *  - Check the test file path to see if the file is the desired data format (e.g. visual inference test checks for image files and audio inference test
 *    checks for audio files). If the file is not desired, simply return a negative integer to tell the caller that this file is skipped.
 *  - Run the test code on the file, if the the file format is desired. If test succeeds, return 0, otherwise return a positive integer to tell the caller
 *    the test failed.
 *  - While running the test code, we also need to know the target test code output (or the content in golden files) so that we can know whether the test is
 *    successful or not. For simple tasks like testing battle menu detector, the target of each test image is a single bool, indicating whether the current
 *    image represents a screenshot with battle menu. So we can embed the target output into the test file name: IngoBattleDayTime_True.png.
 *    For more complex outputs, like detecting how many or where some objects are on the screen, or where the shiny sound is in the audio file, it would
 *    need a very long filename to store them. So we chose to store those outputs in a golden file.
 * 
 *  To avoid implementing the above logic of skipping files and finding target outputs in every TestFunction, we add helper functions like
 *  TestMap.h:int screen_bool_detector_helper(ScreenBoolDetectorFunction test_func, const std::string& test_path)
 *  screen_bool_detector_helper helps implement visual inference test with single bool as output (just like BattleMenuDetector).
 *  It implements skipping non-image files and parse filename to get target output bool.
 *  It then calls ScreenBoolDetectorFunction = std::function<int(const ImageViewRGB32& image, bool target)> test_func which will be implemented by the test developer
 *  to call the actual inference code and check its result against the target bool.
 *  
 *  So in order to implement a test for, let's say, BattleMenuDetector:
 *  - Implement a ScreenBoolDetectorFunction, test_pokemonLA_BattleMenuDetector() that calls the detector code and compares the result with the target bool.
 *    The implementation is written in PokemonLA_Tests.cpp.
 *  - Write the function declaration in PokemonLA_Tests.h
 *  - Add a new entry to TestMap.cpp:TEST_MAP by utilizing screen_bool_detector_helper:
 *    {"PokemonLA_BattleMenuDetector", std::bind(screen_bool_detector_helper, test_pokemonLA_BattleMenuDetector, _1)}
 */


#ifndef PokemonAutomation_Tests_CommandLineTests_H
#define PokemonAutomation_Tests_CommandLineTests_H


namespace PokemonAutomation{


// Called by main() to run tests on command line, without launching any GUI.
// This function is only called when GlobalSettings::COMMAND_LINE_TEST_MODE is true.
// Return 0 if all tests are passed.
int run_command_line_tests();



}
#endif
