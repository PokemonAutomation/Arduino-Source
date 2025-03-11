/*  Game Save
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_GameSave_H
#define PokemonAutomation_PokemonLA_GameSave_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonLA{

// Whether the current tab is the save game tab
bool save_tab_selected(const ImageViewRGB32 &screen);

// Whether the save game tab is disabled: a red "forbidden" synbol appears on the location of the save game tab icon.
bool save_tab_disabled(const ImageViewRGB32 &screen);

// Open menu to save game. Return true if success.
// Return false if in current state the game cannot be saved. This can happen if the player character is in the air
// or is rolling or falling.
bool save_game_from_overworld(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context
);




}
}
}
#endif
