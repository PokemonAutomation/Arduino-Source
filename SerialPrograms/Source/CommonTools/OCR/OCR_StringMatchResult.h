/*  String Match Results
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_StringMatchResult_H
#define PokemonAutomation_CommonTools_OCR_StringMatchResult_H

#include <string>
#include <map>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{
namespace OCR{


struct StringMatchData{
    std::string original_text;
    std::u32string normalized_text;

    std::u32string target;
    std::string token;

    std::string to_str() const;
};

struct StringMatchResult{
    bool exact_match = false;
    std::multimap<double, StringMatchData> results;

    void clear(){
        exact_match = false;
        results.clear();
    }
    void log(Logger& logger, double max_log10p, const std::string& extra = std::string()) const;

    void add(double log10p, StringMatchData data);
    void clear_beyond_spread(double log10p_spread);
    void clear_beyond_log10p(double max_log10p);

    void operator+=(const StringMatchResult& result);
};




}
}
#endif
