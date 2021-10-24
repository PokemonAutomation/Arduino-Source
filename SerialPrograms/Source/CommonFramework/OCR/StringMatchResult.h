/*  String Match Results
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_StringMatchResult_H
#define PokemonAutomation_OCR_StringMatchResult_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <QString>
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{
namespace OCR{


struct StringMatchData{
    QString original_text;
    QString normalized_text;

    QString target;
    std::string token;

    QString to_qstr() const;
};

struct StringMatchResult{
    bool exact_match = false;
    std::multimap<double, StringMatchData> results;
    std::string expected_token;

    void log(Logger& logger, double max_log10p, const QString& extra = QString()) const;

    void add(double log10p, StringMatchData data);
    void clear_beyond_spread(double log10p_spread);
    void clear_beyond_log10p(double max_log10p);
};




}
}
#endif
