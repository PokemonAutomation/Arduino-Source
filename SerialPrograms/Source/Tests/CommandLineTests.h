/*  Command Line Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Explanation of the command line test framework
 * 
 *  Goal:
 *  - To build a unit test-like framework to ensure correctness of the various inference methods.
 *  - To allow efficient iterations during development of a new inference method without the need to launch GUI and click buttons.
 * 
 *  How to use:
 *  Enable the command line test mode by changing the SerialPrograms-Settings.json field:
 *  "20-GlobalSettings": "COMMAND_LINE_TESTS": "RUN" to true.
 *  In this mode, the program does not launch GUI. Instead it looks for a local folder ./CommandLineTests/ to run tests.
 *  The folder is structured as:
 *  CommandLineTests/                          <- root test folder
 *    - PokemonLA/                               <- test space, sub-folder to organize tests, e.g. by which games the inferences are used in
 *        - BattleMenuDetector/                    <- test object, name of the class/function/file to test
 *            - IngoBattleDayTime-True.png           <- test file for BattleMenuDetector
 *            - IngoBattleNightTime-True.png         <- test file for BattleMenuDetector
 *    - PokemonBDSP/                             <- another test space for inferences in another game
 *        - DialogDetector/                        <- test object, this time it's DialogDetector for BDSP
 *            - Win_Mirabox/                         <- can have more folders under test object to organize test files, e.g by OS and capture card
 *                - FetchEggDayTime-True.png           <- test file for DialogDetector
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
 *  - "PokemonBDSP/DialogDetector/Win_Mirabox/FetchEggDayTime-True.png"
 *  This gives the flexibility to test the code for a game, a detector, a detector on a capture card or a detector on a particular image/audio/video.
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
