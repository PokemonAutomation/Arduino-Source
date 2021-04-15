/*  Shiny Hunt Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ShinyHuntTracker.h"

namespace PokemonAutomation{




ShinyHuntTracker::ShinyHuntTracker(bool shiny_types)
    : m_encounters(m_stats["Encounters"])
    , m_unknown_shinies(m_stats["Unknown Shinies"])
    , m_star_shinies(m_stats["Star Shinies"])
    , m_square_shinies(m_stats["Square Shinies"])
{
    m_display_order.emplace_back("Encounters");
    if (shiny_types){
        m_display_order.emplace_back("Star Shinies");
        m_display_order.emplace_back("Square Shinies");
        m_display_order.emplace_back("Unknown Shinies", true);
    }else{
        m_display_order.emplace_back("Shinies");
    }
}
void ShinyHuntTracker::add_non_shiny(){
    m_encounters++;
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




}
