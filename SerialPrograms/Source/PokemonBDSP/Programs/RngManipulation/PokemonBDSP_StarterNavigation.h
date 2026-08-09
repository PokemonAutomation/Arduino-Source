/*  BDSP Starter Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StarterNavigation_H
#define PokemonAutomation_PokemonBDSP_StarterNavigation_H

#include <stdint.h>
#include <cmath>
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


std::vector<BdspEyeTemplate> lake_eye_templates(uint8_t player_model);

std::vector<BdspEyeTemplate> bedroom_eye_templates(uint8_t player_model);

uint8_t starter_cursor_steps(BdspStarter starter);
const char* starter_slug(BdspStarter starter);


//  Measured over 14 attempts spanning 40k to 734k advances, +/-0.01%
const double BDSP_SKIP_ADVANCES_PER_MINUTE = 19653;

//  Measured over the same attempts, +/-29
const double BDSP_SKIP_NAVIGATION_ADVANCES = 6902;

//  How far short of the target to stop quickly skipping advances
const double BDSP_SKIP_BUFFER_BASE = 750;
const double BDSP_SKIP_BUFFER_PER_ADVANCE = 0.0002;
inline uint64_t bdsp_skip_buffer(double advances_skipped){
    return (uint64_t)(BDSP_SKIP_BUFFER_BASE + BDSP_SKIP_BUFFER_PER_ADVANCE * advances_skipped);
}


const double BDSP_SKIP_RATE_SIGMA = 0.0001;
const double BDSP_SKIP_SCATTER_SIGMA = 67;
inline double bdsp_skip_landing_sigma(double advances_skipped){
    double from_rate = BDSP_SKIP_RATE_SIGMA * advances_skipped;
    return std::sqrt(from_rate * from_rate + BDSP_SKIP_SCATTER_SIGMA * BDSP_SKIP_SCATTER_SIGMA);
}


inline uint64_t bdsp_lake_advances_needed(double advances_skipped){
    return bdsp_skip_buffer(advances_skipped)
        + (uint64_t)(2 * bdsp_skip_landing_sigma(advances_skipped));
}


void navigate_bedroom_to_skip_spot(Logger& logger, ProControllerContext& context);

void navigate_skip_spot_to_lakefront(Logger& logger, ProControllerContext& context);


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
bool clear_starter_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

double seconds_to_move_to_starter(BdspStarter starter);


double seconds_from_select_to_confirm();


double seconds_from_briefcase_to_starly_ready();


double seconds_to_pre_briefcase();


}
}
}
#endif
