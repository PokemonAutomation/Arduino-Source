/*  Image Match Result
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ImageMatchResult.h"

namespace PokemonAutomation{
namespace ImageMatch{


void MatchResult::log(Logger& logger, double max_RMSD) const{
    std::string str = "Image Match Result: ";

    if (slugs.empty()){
        str += "no matches";
        logger.log(str, Qt::red);
        return;
    }

    double best = slugs.begin()->first;
    QColor color = best <= max_RMSD
        ? Qt::blue
        : Qt::red;

    if (slugs.size() == 1){
        auto iter = slugs.begin();
        str += iter->second;
        str += " (RMSD = ";
        str += std::to_string(iter->first);
        str += ")";
        logger.log(str, color);
        return;
    }

    str += "Multiple Candidates =>\n";
    size_t printed = 0;
    for (const auto& item : slugs){
        if (printed == 10){
            str += "    (" + std::to_string(slugs.size() - 10) + " more...)\n";
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



}
}
