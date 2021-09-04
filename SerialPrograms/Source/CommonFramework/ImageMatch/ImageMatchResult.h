/*  Image Match Result
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageMatchResult_H
#define PokemonAutomation_CommonFramework_ImageMatchResult_H

#include <string>
#include <map>
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{
namespace ImageMatch{


struct MatchResult{
    std::multimap<double, std::string> slugs;

    void log(Logger& logger, double max_RMSD) const;
};


}
}
#endif
