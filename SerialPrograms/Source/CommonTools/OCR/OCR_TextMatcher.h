/*  Text Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_TextMatcher_H
#define PokemonAutomation_CommonTools_OCR_TextMatcher_H

#include <string>
#include <set>
#include <map>
#include <QString>
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


size_t levenshtein_distance(const QString& x, const QString& y);
size_t levenshtein_distance_substring(const QString& substring, const QString& fullstring);

template <typename StringType>
size_t levenshtein_distance(const StringType& x, const StringType& y);
template <typename StringType>
size_t levenshtein_distance_substring(const StringType& substring, const StringType& fullstring);

//  Mathematically equivalent to:
//      BinomialCDF[total, 1 - random_match_chance, total - matched]
double random_match_probability(size_t total, size_t matched, double random_match_chance);



StringMatchResult match_substring(
    const std::map<std::u32string, std::set<std::string>>& database, double random_match_chance,
    const std::string& text, double log10p_spread
);




}
}
#endif
