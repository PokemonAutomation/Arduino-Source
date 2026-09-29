/*  BDSP Blink Extraction
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BlinkExtraction_H
#define PokemonAutomation_PokemonBDSP_BlinkExtraction_H

#include <stddef.h>
#include <string>
#include <vector>
#include "PokemonBDSP/Programs/RngManipulation/PokemonBDSP_BlinkModel.h"
#include "PokemonBDSP/Programs/RngManipulation/PokemonBDSP_StateSolver.h"
#include "PokemonBDSP_EyeBlinkDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct Blink{
    double seconds = 0;
    double depth = 0;
    size_t frames = 0;
};

struct BlinkEvent{
    double seconds = 0;
    BlinkType type = BlinkType::Single;
};

struct TickFit{
    double period_seconds = 0;
    double rms_ticks = 0;
    double worst_ticks = 0;
};


//  How far the match has to fall below its resting level to count as a blink
double auto_blink_threshold(const std::vector<BlinkMatchSample>& samples);


std::vector<Blink> extract_blinks(
    const std::vector<BlinkMatchSample>& samples, double threshold,
    WallClock origin = WallClock::min()
);

//  A second blink arriving within "double_blink_seconds" is the same roll
std::vector<BlinkEvent> group_blinks(
    const std::vector<Blink>& blinks,
    double double_blink_seconds = 0.55
);

//  The NPC tick is nominally 1.017 s but measures 1.0197 s on real hardware
TickFit fit_tick_period(
    const std::vector<BlinkEvent>& events,
    double lowest_seconds = 1.000,
    double highest_seconds = 1.040
);

bool build_samples(
    const std::vector<std::vector<BlinkEvent>>& streams,
    const std::vector<uint8_t>& slots,
    double period_seconds,
    std::vector<BlinkSample>& samples,
    std::string& failure_reason
);

bool last_blink_anchor(
    const std::vector<std::vector<BlinkEvent>>& streams,
    const std::vector<uint8_t>& slots,
    double period_seconds,
    uint64_t& advance,
    double& seconds,
    size_t& stream_index
);


//  Move an anchor forward onto a later blink from the same stream.
bool step_blink_anchor(
    double elapsed_seconds,
    double period_seconds,
    uint8_t npcs,
    uint64_t previous_advance,
    uint64_t& advance
);


}
}
}
#endif
