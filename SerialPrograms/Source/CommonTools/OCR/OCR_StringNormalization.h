/*  String Normalization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_StringNormalization_H
#define PokemonAutomation_CommonTools_OCR_StringNormalization_H

#include <string>

namespace PokemonAutomation{
namespace OCR{


// Return only digits and word characters (no punctuation, whitespace or math operators).
// This is called by `normalize_utf32()` as part of text processing for OCR matching
// against a dictionary.
std::u32string remove_non_alphanumeric(const std::u32string& text);

// Apply character substitution rules loaded from RESOURCE_PATH()/Tesseract/CharacterReductions.json
// to merge similar-looking characters. This is called by `normalize_utf32()` as a fix for OCR
// failure on those similar-looking characters.
std::u32string run_character_reductions(const std::u32string& text);

// Convert UTF-8 string to UTF-32 and normalize it to make OCR matching against
// a dictionary easier:
// 1. Appy Unocide Compatibility Decomposition using QString, which decomposes
//    each unicode into smaller easy-to-search components, e.g. '½' becomes "1/2".
// 2. Call `remove_non_alphanumeric()` to remove punctuation, whitespace or math operators
//    that are not part of any words or digits.
// 3. Call `run_character_reductions()` to merge similar-look characters.
// 4. Convert all uppercase letters to lowercase.
std::u32string normalize_utf32(const std::string& text);



}
}
#endif
