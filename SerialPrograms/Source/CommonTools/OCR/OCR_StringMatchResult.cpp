/*  String Match Results
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/StringToolsQt.h"
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


std::string StringMatchData::to_str() const{
    std::string str;

    str += "\"";
    for (char ch : original_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    str += "\" -> ";
    str += "\"" + to_utf8(normalized_text) + "\" -> ";

    str += "(" + to_utf8(target) + "): ";
    str += "(" + token + ")";
    return str;
}

void StringMatchResult::log(Logger& logger, double max_log10p, const std::string& extra) const{
    std::string str = "String Match Result: ";

#if 0
    if (!expected_token.empty()){
        str += "Expected (";
        str += QString::fromStdString(expected_token);
        str += "): ";
    }
#endif

    Color color;
    if (results.empty()){
        str += "no matches";
        color = COLOR_RED;
    }else{
        double best = results.begin()->first;
        color = exact_match || best <= max_log10p
            ? COLOR_BLUE
            : COLOR_RED;

        if (results.size() == 1){
            auto iter = results.begin();
            str += iter->second.to_str();
            str += " (log10p = " + tostr_default(iter->first) +")";
        }else{
            str += "Multiple Candidates =>\n";
            size_t printed = 0;
            for (const auto& item : results){
                if (printed == 10){
                    str += "    (" + std::to_string(results.size() - 10) + " more...)\n";
                    break;
                }
                str += "    " + tostr_default(item.first) + " : " + item.second.to_str() + "\n";
                printed++;
            }
        }
    }

    if (!extra.empty()){
        str += " ===> ";
        str += extra;
    }
    logger.log(str, color);
}

void StringMatchResult::add(double log10p, StringMatchData data){
    results.emplace(log10p, std::move(data));
}
void StringMatchResult::clear_beyond_spread(double log10p_spread){
    auto best = results.begin();
    while (results.size() > 1){
        auto back = results.rbegin();
        if (back->first <= best->first + log10p_spread){
            break;
        }
        results.erase(back->first);
    }
}
void StringMatchResult::clear_beyond_log10p(double max_log10p){
    while (!results.empty()){
        auto iter = results.end();
        --iter;
        if (iter->first <= max_log10p){
            break;
        }
        results.erase(iter);
    }
}


void StringMatchResult::operator+=(const StringMatchResult& result){
    exact_match |= result.exact_match;
    for (auto& item : result.results){
        results.insert(item);
    }
}




}
}
