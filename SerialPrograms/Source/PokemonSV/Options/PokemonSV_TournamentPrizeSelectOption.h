/*  Tournament Prize Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TournamentPrizeSelectOption_H
#define PokemonAutomation_PokemonSV_TournamentPrizeSelectOption_H

#include "CommonFramework/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class TournamentPrizeSelectCell : public StringSelectCell{
public:
    TournamentPrizeSelectCell(const std::string& default_slug);
};

}
}
}
#endif
