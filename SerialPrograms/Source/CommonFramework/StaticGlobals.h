/*  Static Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StaticGlobals_H
#define PokemonAutomation_StaticGlobals_H

#include <stdint.h>

namespace PokemonAutomation{

class JsonValue;


//
//  These are settings that are needed very early in the boot process.
//  These do not come with UI and are loaded/saved manually (if required).
//

class StaticGlobals{
public:
    void load_json(const JsonValue& json);
    JsonValue to_json_debug() const;

public:
    bool NAUGHTY_MODE = false;
    bool DEVELOPER_MODE = false;
    bool INTERNAL_DEVELOPER_MODE = false;


    // Debug color stats like those in:
    // - CommonFramework/ImageTools/ImageStats.cpp
    bool COLOR_CHECK = false;
    // Debug image tempalte matching like those in:
    // - CommonTools/ImageMatch/WaterfillTemplateMatcher.cpp
    bool IMAGE_TEMPLATE_MATCHING = false;
    // Debug image dictionary matching like those in:
    // - CommonTools/ImageMatch/CroppedImageDictionaryMatcher.cpp
    bool IMAGE_DICTIONARY_MATCHING = false;
    // Debug box system detection to only debug on a box cell at row:
    // - SerialPrograms/Source/PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.cpp
    int8_t BOX_SYSTEM_CELL_ROW = -1;
    // Debug box system detection to only debug on a box cell at col:
    // - SerialPrograms/Source/PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.cpp
    // - SerialPrograms/Source/PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.cpp
    int8_t BOX_SYSTEM_CELL_COL = -1;
    // A golden file is a file in software testing that contains the exact, expected output of the
    // test code.
    // If true, this bool changes some test functions' behavior from running the tests against golden
    // files to generating golden files by writing detection outputs into them.
    // This switch is useful for making tests for lots of detections on an image, e.g. menu item name
    // OCR or outbreak/event detection on a map. It is time consuming and error-prone to write each
    // expected result into a file manually. Instead, run the detection code to generate golden files
    // then manually inspect to fix any errors.
    bool GENERATE_TEST_GOLDEN_FILES = false;


};


extern StaticGlobals STATIC_GLOBALS;



}
#endif
