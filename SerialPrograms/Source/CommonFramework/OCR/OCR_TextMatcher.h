/*  Text Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_TextMatcher_H
#define PokemonAutomation_OCR_TextMatcher_H

#include <vector>
#include <set>
#include <map>
#include <QString>
#include "CommonFramework/Tools/Logger.h"
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


size_t levenshtein_distance(const QString& x, const QString& y);
size_t levenshtein_distance_substring(const QString& substring, const QString& fullstring);

//  Mathematically equivalent to:
//      BinomialCDF[total, 1 - random_match_chance, total - matched]
double random_match_probability(size_t total, size_t matched, double random_match_chance);



void match_substring(
    StringMatchResult& results,
    const std::map<QString, std::set<std::string>>& database, double random_match_chance,
    const QString& text, double log10p_spread
);




}
}
#endif
