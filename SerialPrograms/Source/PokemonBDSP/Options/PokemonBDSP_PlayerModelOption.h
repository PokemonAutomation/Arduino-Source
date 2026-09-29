/*  Player Model Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_PlayerModelOption_H
#define PokemonAutomation_PokemonBDSP_PlayerModelOption_H

#include <stdint.h>
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  The eight appearances the player character can be given at the start of the game.
const uint8_t BDSP_PLAYER_MODEL_COUNT = 8;


class PlayerModelOption : public StringSelectOption{
public:
    PlayerModelOption();
    uint8_t model_number() const{ return (uint8_t)(index() + 1); }
};


}
}
}
#endif
