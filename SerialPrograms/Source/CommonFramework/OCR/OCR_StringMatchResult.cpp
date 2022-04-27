/*  String Match Results
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


QString StringMatchData::to_qstr() const{
    QString str;

    str += "\"";
    for (QChar ch : original_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    str += "\" -> ";
    str += "\"" + normalized_text + "\" -> ";

    str += "(" + target + "): ";
    str += "(" + QString::fromStdString(token) + ")";
    return str;
}

void StringMatchResult::log(LoggerQt& logger, double max_log10p, const QString& extra) const{
    QString str = "String Match Result: ";

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
            str += iter->second.to_qstr();
            str += " (log10p = ";
            str += QString::number(iter->first);
            str += ")";
        }else{
            str += "Multiple Candidates =>\n";
            size_t printed = 0;
            for (const auto& item : results){
                if (printed == 10){
                    str += "    (" + QString::number(results.size() - 10) + " more...)\n";
                    break;
                }
                str += "    ";
                str += QString::number(item.first);
                str += " : ";
                str += item.second.to_qstr();
                str += "\n";
                printed++;
            }
        }
    }

    if (!extra.isEmpty()){
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




}
}
