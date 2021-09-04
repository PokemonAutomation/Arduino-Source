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

namespace PokemonAutomation{
namespace OCR{



size_t levenshtein_distance(const QString& x, const QString& y);
size_t levenshtein_distance_substring(const QString& substring, const QString& fullstring);

//  Mathematically equivalent to:
//      BinomialCDF[total, 1 - random_match_chance, total - matched]
double random_match_probability(size_t total, size_t matched, double random_match_chance);

struct MatchResult{
    bool matched = false;
//    double exact_match_error = 1.0;
    double alpha = 0;
    QString ocr_text;
    QString normalized_text;
    std::vector<QString> candidates;
    std::set<std::string> slugs;
    std::string expected_token;

    void log(Logger& logger, const QString& extra = QString()) const;
};


MatchResult match_substring(
    const std::map<QString, std::set<std::string>>& database,
    const QString& text,
    double random_match_chance,
    double min_alpha
);




}
}
#endif
