/*  Shiny Hunt Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ShinyHuntTracker_H
#define PokemonAutomation_ShinyHuntTracker_H

#include "CommonFramework/Tools/StatsTracking.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class ShinyHuntTracker : public StatsTracker{
public:
    ShinyHuntTracker(bool shiny_types);
    ShinyHuntTracker(bool shiny_types, std::map<std::string, std::string> aliases);

    uint64_t encounters() const{ return m_encounters; }

    void operator+=(ShinyType detection);
    void add_non_shiny();
    void add_error();
    void add_unknown_shiny();
    void add_star_shiny();
    void add_square_shiny();

private:
    uint64_t& m_encounters;
    uint64_t& m_errors;

    uint64_t& m_unknown_shinies;
    uint64_t& m_star_shinies;
    uint64_t& m_square_shinies;
};



}
}
}
#endif
