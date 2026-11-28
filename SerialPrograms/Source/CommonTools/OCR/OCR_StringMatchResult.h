/*  String Match Results
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_StringMatchResult_H
#define PokemonAutomation_CommonTools_OCR_StringMatchResult_H

#include <string>
#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"

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
    // Write to log the current matching results stored in `results`, e.g.
    //   String Match Result: "Lonely" -> "10ne1y" -> (10ne1y): (Lonely) (log10p = -4.19382)
    // Use `max_log10p` to choose the color: blue if it is a match, red if not.
    // `extra` is added to the end of the line.
    // It logs at `LogLevel::Verbose`, since programs read text many times in a run.
    void log(Logger& logger, double max_log10p, const std::string& extra = std::string()) const;

    void add(double log10p, StringMatchData data);
    void clear_beyond_spread(double log10p_spread);
    void clear_beyond_log10p(double max_log10p);

    void operator+=(const StringMatchResult& result);
};




}
}
#endif
