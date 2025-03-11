/*  Shiny Hunt Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "ShinyHuntTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




ShinyHuntTracker::ShinyHuntTracker(bool shiny_types)
    : m_encounters(m_stats["Encounters"])
    , m_caught(m_stats["Caught"])
    , m_errors(m_stats["Errors"])
    , m_unknown_shinies(m_stats[shiny_types ? "Unknown Shinies" : "Shinies"])
    , m_star_shinies(m_stats["Star Shinies"])
    , m_square_shinies(m_stats["Square Shinies"])
{
    m_display_order.emplace_back("Encounters");
    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    if (shiny_types){
        m_display_order.emplace_back("Star Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Square Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Unknown Shinies", HIDDEN_IF_ZERO);
    }else{
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    m_display_order.emplace_back("Caught", HIDDEN_IF_ZERO);
}
ShinyHuntTracker::ShinyHuntTracker(bool shiny_types, std::map<std::string, std::string> aliases)
    : ShinyHuntTracker(shiny_types)
{
    m_aliases = std::move(aliases);
}

void ShinyHuntTracker::operator+=(ShinyType detection){
    switch (detection){
    case ShinyType::UNKNOWN:
        m_errors++;
        break;
    case ShinyType::NOT_SHINY:
        m_encounters++;
        break;
    case ShinyType::MAYBE_SHINY:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add a MAYBE_SHINY to stats.");
    case ShinyType::UNKNOWN_SHINY:
        m_encounters++;
        m_unknown_shinies++;
        break;
    case ShinyType::STAR_SHINY:
        m_encounters++;
        m_star_shinies++;
        break;
    case ShinyType::SQUARE_SHINY:
        m_encounters++;
        m_square_shinies++;
        break;
    }
}
void ShinyHuntTracker::add_non_shiny(){
    m_encounters++;
}
void ShinyHuntTracker::add_error(){
    m_errors++;
}
void ShinyHuntTracker::add_unknown_shiny(){
    m_encounters++;
    m_unknown_shinies++;
}
void ShinyHuntTracker::add_star_shiny(){
    m_encounters++;
    m_star_shinies++;
}
void ShinyHuntTracker::add_square_shiny(){
    m_encounters++;
    m_square_shinies++;
}
void ShinyHuntTracker::add_caught(){
    m_caught++;
}




}
}
}
