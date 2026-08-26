/*  BDSP Blink Recovery
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BlinkRecovery_H
#define PokemonAutomation_PokemonBDSP_BlinkRecovery_H

#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/InferenceCallbacks/InferenceCallback.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_BlinkExtraction.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_EyeBlinkDetector.h"
#include "PokemonBDSP_AdvanceClock.h"
#include "PokemonBDSP_RngDisplays.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{



struct BlinkRecoveryConfig{
    uint8_t npcs = 2;
    size_t min_rolls_to_try = 44;
    size_t confirmation_events = 6;
    size_t window_events = 52; // chosen for redundancy
    size_t max_failed_windows = 3;
    Seconds liveness_timeout = std::chrono::seconds(3600); // in case blinks are never arriving
    Seconds poll_interval = std::chrono::seconds(1);
    Seconds reanchor_interval = std::chrono::seconds(30);
    Seconds keep_awake_interval = std::chrono::seconds(180);
    Seconds blink_retention = std::chrono::seconds(180);
    size_t reanchor_blinks = 6;
    uint64_t reanchor_search_radius = 600;
    size_t max_reanchor_failures = 4;
};


struct BlinkRecovery{
    bool success = false;
    Pokemon::Xorshift128State state;
    AdvanceClock clock;
    size_t events = 0;
    std::string failure_reason;
    std::vector<double> thresholds;
    std::vector<uint8_t> slots;
    size_t anchor_stream = 0;
};



struct BlinkSceneConfig{
    std::function<bool(const ImageViewRGB32& frame)> frame_filter;
    double minimum_match = 0.35;
    size_t required_streak = 4;
    Seconds attempt_timeout = std::chrono::seconds(10);
    size_t press_retries = 10;
    Button retry_button = BUTTON_A;
};


std::vector<ImageRGB32> load_eye_templates(
    const std::vector<BdspEyeTemplate>& setups
);

bool wait_for_blink_scene(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const std::vector<BdspEyeTemplate>& setups,
    const std::vector<ImageRGB32>& eyes,
    const BlinkSceneConfig& config,
    const std::string& scene_name
);

void make_blink_watchers(
    const std::vector<BdspEyeTemplate>& setups,
    const std::vector<ImageRGB32>& eyes,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks
);


bool collect_blink_matches(
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<std::vector<BlinkMatchSample>>& matches,
    WallClock& origin
);


std::vector<std::vector<BlinkEvent>> build_blink_streams(
    const std::vector<std::vector<BlinkMatchSample>>& matches,
    const std::vector<double>& thresholds,
    WallClock origin
);


void keep_awake_if_due(ProControllerContext& context, WallClock& next, Seconds interval);


BlinkRecovery recover_state_from_blinks(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks,
    BlinkCollectionDisplay& display,
    const BlinkRecoveryConfig& config
);


// re-derive the newest blink's advance from the recovered state
bool reanchor_absolute(
    const BlinkRecovery& recovery,
    const std::vector<std::vector<BlinkEvent>>& streams,
    WallClock origin,
    const BlinkRecoveryConfig& config,
    AdvanceClock& clock,
    Logger& logger
);

// hold position, re-anchoring periodically, until the press is close enough to walk to
void hold_and_reanchor(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::unique_ptr<EyeBlinkWatcher>>& watchers,
    std::vector<PeriodicInferenceCallback>& callbacks,
    BlinkRecovery& recovery,
    const BlinkRecoveryConfig& config,
    uint64_t press_advance,
    double lead_seconds,
    RngStateDisplay& display
);


}
}
}
#endif
