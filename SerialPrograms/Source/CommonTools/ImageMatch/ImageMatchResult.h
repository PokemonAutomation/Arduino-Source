/*  Image Match Result
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_ImageMatchResult_H
#define PokemonAutomation_CommonTools_ImageMatchResult_H

#include <string>
#include <map>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{
namespace ImageMatch{


struct ImageMatchResult{
    // Stored match score (the lower the closer the match) -> slug of the image template that gives the score.
    std::multimap<double, std::string> results;

    // Write to log the current matching results stored in `results`.
    // Use `max_alpha` to determine whether matches are found.
    void log(Logger& logger, double max_alpha) const;

    // Add to `results`.
    // alpha: the match score, the lower the closer the match.
    // slug: slug of the image template that gives the score.
    void add(double alpha, std::string slug);

    // Assume there is at least one result in `results`:
    // Remove other weaker match results that have a score that's larger than the best match score + `max_alpha_spread`.
    void clear_beyond_spread(double max_alpha_spread);
    // Remove match results with scores > `max_alpha`.
    void clear_beyond_alpha(double max_alpha);
};


}
}
#endif
