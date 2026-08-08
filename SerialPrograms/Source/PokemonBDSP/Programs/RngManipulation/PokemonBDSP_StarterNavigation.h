/*  BDSP Starter Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StarterNavigation_H
#define PokemonAutomation_PokemonBDSP_StarterNavigation_H

#include <stdint.h>
#include <string>
#include <vector>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_EyeBlinkDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BdspStarter{
    Turtwig,
    Chimchar,
    Piplup,
};

enum class BdspPlayerModel{
    Model1,
    Model4,
};


//  BdspEyeTemplate lives in PokemonBDSP_EyeBlinkDetector.h, since every blink scene
//  supplies one and the recovery machinery does not care which scene it came from.
std::vector<BdspEyeTemplate> lake_eye_templates(BdspPlayerModel model);

uint8_t starter_cursor_steps(BdspStarter starter);
const char* starter_slug(BdspStarter starter);


void navigate_to_lake_blinks(Logger& logger, ProControllerContext& context);

void navigate_to_pre_briefcase(Logger& logger, ProControllerContext& context);


bool issue_starter_sequence(
    ProControllerContext& context,
    BdspStarter starter,
    double starly_seconds,
    double select_seconds,
    double confirm_seconds,
    std::string& failure_reason
);


//  Mash A until the battle with the wild Starly is over.
//
//  It cannot realistically be lost — a level 5 starter against a level 2 Starly,
//  with the rival's Pokemon alongside — so there is no losing branch to handle.
//  Returns false if the battle never ended, which means something other than a
//  battle was on screen.
bool clear_starter_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

double seconds_to_move_to_starter(BdspStarter starter);


double seconds_from_select_to_confirm();


double seconds_from_briefcase_to_starly_ready();


double seconds_to_pre_briefcase();


}
}
}
#endif
