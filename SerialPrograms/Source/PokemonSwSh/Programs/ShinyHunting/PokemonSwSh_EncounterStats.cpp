/*  Encounter Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "PokemonSwSh_EncounterStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


EncounterStats::EncounterStats(bool shiny_types)
    : m_shiny_types(shiny_types)
{}

void EncounterStats::log_stats(ProgramEnvironment& env, Logger& logger) const{
    QString str = "<b>" + QString(stats().c_str()) + "</b>";
    env.set_status(str);
    logger.log(str);
}
std::string EncounterStats::stats() const{
    std::string str;
    str += str_encounters();
    str += str_shinies();
    return str;
}
std::string EncounterStats::str_encounters() const{
    std::string str;
    str += "Encounters: " + tostr_u_commas(m_encounters);
    return str;
}
std::string EncounterStats::str_shinies() const{
    std::string str;
    if (m_shiny_types){
        str += " - Star Shinies: " + tostr_u_commas(m_star_shinies);
        str += " - Square Shinies: " + tostr_u_commas(m_square_shinies);
    }else{
        str += " - Shinies: " + tostr_u_commas(m_shinies);
    }
    return str;
}



void EncounterStats::add_non_shiny(){
    m_encounters++;
}
void EncounterStats::add_shiny(){
    m_encounters++;
    m_shinies++;
}
void EncounterStats::add_star_shiny(){
    m_encounters++;
    m_shinies++;
    m_star_shinies++;
}
void EncounterStats::add_square_shiny(){
    m_encounters++;
    m_shinies++;
    m_square_shinies++;
}



}
}
}

