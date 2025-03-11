/*  Tournament Prize Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TournamentPrizeNames_H
#define PokemonAutomation_PokemonSV_TournamentPrizeNames_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class TournamentPrizeNames{
public:
    const std::string& display_name() const { return m_display_name; }

private:
    friend struct TournamentPrizeNameDatabase;

    std::string m_display_name;
};


const TournamentPrizeNames& get_tournament_prize_name(const std::string& slug);
const std::string& parse_tournament_prize_name(const std::string& display_name);
const std::string& parse_tournament_prize_name_nothrow(const std::string& display_name);

const std::vector<std::string>& TOURNAMENT_PRIZE_SLUGS();


}
}
}
#endif
