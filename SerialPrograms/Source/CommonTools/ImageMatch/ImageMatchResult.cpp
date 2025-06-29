/*  Image Match Result
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ImageMatchResult.h"

namespace PokemonAutomation{
namespace ImageMatch{


void ImageMatchResult::log(Logger& logger, double max_alpha) const{
    std::string str = "Image Match Result: ";

    if (results.empty()){
        str += "no matches";
        logger.log(str, COLOR_RED);
        return;
    }

    double best = results.begin()->first;
    Color color = best <= max_alpha
        ? COLOR_BLUE
        : COLOR_RED;

    if (results.size() == 1){
        auto iter = results.begin();
        str += iter->second;
        str += " (alpha = ";
        str += std::to_string(iter->first);
        str += ")";
        logger.log(str, color);
        return;
    }

    str += "Multiple Candidates =>\n";
    size_t printed = 0;
    for (const auto& item : results){
        if (printed == 10){
            str += "    (" + std::to_string(results.size() - 10) + " more...)\n";
            break;
        }
        str += "    ";
        str += std::to_string(item.first);
        str += " : ";
        str += item.second;
        str += "\n";
        printed++;
    }

    logger.log(str, color);
}

void ImageMatchResult::add(double alpha, std::string slug){
    results.emplace(alpha, std::move(slug));
}
void ImageMatchResult::clear_beyond_spread(double max_alpha_spread){
    auto best = results.begin();
    while (results.size() > 1){
        auto back = results.end();
        --back;
        if (back->first <= best->first + max_alpha_spread){
            break;
        }
        results.erase(back);
    }
}
void ImageMatchResult::clear_beyond_alpha(double max_alpha){
    while (!results.empty()){
        auto iter = results.end();
        --iter;
        if (iter->first <= max_alpha){
            break;
        }
        results.erase(iter);
    }
}




}
}
