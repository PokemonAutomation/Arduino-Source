/*  Language
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Language enum and utilities for managing game languages in Pokemon and Nintendo Switch automation.
 *  Supports OCR, game text detection, and multi-language configurations.
 */

#ifndef PokemonAutomation_Language_H
#define PokemonAutomation_Language_H

#include <string>
#include <set>

namespace PokemonAutomation{

// Supported game languages for OCR and text detection
enum class Language{
    None,
    English,
    Japanese,
    Spanish,
    French,
    German,
    Italian,
    Korean,
    ChineseSimplified,
    ChineseTraditional,
    EndOfList,
};

// Language metadata containing language code, display name, and OCR false-positive probability
struct LanguageData{
    // Tesseract OCR language code (e.g., "eng", "jpn")
    // See SerialPrograms/Source/CommonFramework/Language.cpp:LANGUAGE_DATA for complete
    // list of codes.
    std::string code;
    // Human-readable language name
    std::string name;
    // Probability of false positive OCR match. Range [0.0, 1.0].
    // The lower the value the less chance an OCR detection is false positive.
    double random_match_chance;
};

// std::set-based container for specifying multiple languages.
class LanguageSet{
public:
    LanguageSet() = default;
    LanguageSet(std::initializer_list<Language> list)
        : m_set(list)
    {}

    bool operator[](Language language) const;   // Check if language is in set
    void operator+=(Language language);          // Add language to set
    void operator-=(Language language);          // Remove language from set
    void operator+=(const LanguageSet& set);     // Union with another set
    void operator-=(const LanguageSet& set);     // Difference with another set

    std::set<Language>::const_iterator begin() const{
        return m_set.begin();
    }
    std::set<Language>::const_iterator end() const{
        return m_set.end();
    }


private:
    std::set<Language> m_set;
};

// Retrieve language metadata `LanguageData` for a given Language enum
const LanguageData& language_data(Language language);

// Convert language code string to Language enum.
// See SerialPrograms/Source/CommonFramework/Language.cpp:LANGUAGE_DATA for complete
// list of language codes
Language language_code_to_enum(const std::string& language);

// Generate user-facing warning message about language configuration:
// "Please ensure that you have set the correct Game Language in the program settings"
// "Current Language Set: <language>".
std::string language_warning(Language language);



}
#endif
