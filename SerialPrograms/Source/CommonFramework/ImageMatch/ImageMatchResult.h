/*  Image Match Result
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageMatchResult_H
#define PokemonAutomation_CommonFramework_ImageMatchResult_H

#include <string>
#include <map>
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{
namespace ImageMatch{


struct ImageMatchResult{
    std::multimap<double, std::string> results;

    void log(Logger& logger, double max_alpha) const;

    void add(double alpha, std::string slug);
    void clear_beyond_spread(double max_alpha_spread);
    void clear_beyond_alpha(double max_alpha);
};


}
}
#endif
