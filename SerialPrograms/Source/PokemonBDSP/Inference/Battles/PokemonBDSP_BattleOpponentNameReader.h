/*  BDSP Battle Opponent Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  OCR-based detection of opponent pokemon names from the BDSP battle screen.
 *  Used by the outside-help battle handler to track which pokemon are currently
 *  on the field and update move selection accordingly.
 *
 *  ⚠  Screen region coordinates are estimates and may require calibration
 *     against a real BDSP capture before they work reliably in production.
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleOpponentNameReader_H
#define PokemonAutomation_PokemonBDSP_BattleOpponentNameReader_H

#include <string>
#include <vector>
#include "CommonFramework/Language.h"
#include "PokemonBDSP/Programs/AutoStory/Data/PokemonBDSP_AutoStory_BDSPData.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Read the opponent pokemon name boxes at the top of the BDSP battle screen.
//
//  Returns one slug per opponent slot (up to 1 for singles, 2 for doubles).
//  An empty string in a slot means OCR confidence was too low to identify the pokemon.
//  The caller should keep the previous state for any slot that returns empty string.
std::vector<std::string> read_battle_opponent_names(
    Logger& logger,
    Language language,
    const ImageViewRGB32& frame,
    BDSPBattleType battle_type
);


}
}
}
#endif
