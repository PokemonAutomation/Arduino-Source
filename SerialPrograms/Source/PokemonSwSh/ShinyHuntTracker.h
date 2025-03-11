/*  Shiny Hunt Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ShinyHuntTracker_H
#define PokemonAutomation_ShinyHuntTracker_H

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "Pokemon/Pokemon_DataTypes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class ShinyHuntTracker : public StatsTracker{
public:
    ShinyHuntTracker(bool shiny_types);
    ShinyHuntTracker(bool shiny_types, std::map<std::string, std::string> aliases);

    uint64_t encounters() const{ return m_encounters.load(std::memory_order_relaxed); }

    void operator+=(ShinyType detection);
    void add_non_shiny();
    void add_error();
    void add_unknown_shiny();
    void add_star_shiny();
    void add_square_shiny();
    void add_caught();

private:
    std::atomic<uint64_t>& m_encounters;
    std::atomic<uint64_t>& m_caught;
    std::atomic<uint64_t>& m_errors;

    std::atomic<uint64_t>& m_unknown_shinies;
    std::atomic<uint64_t>& m_star_shinies;
    std::atomic<uint64_t>& m_square_shinies;
};



}
}
}
#endif
