/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H

#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP_ShinySparkleSet.h"

namespace PokemonAutomation{
    class CancellableScope;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;



struct DoublesShinyDetection : public ShinyDetectionResult{
    bool left_is_shiny = false;
    bool right_is_shiny = false;
};


struct DetectionType{
    ImageFloatBox box;
    PokemonSwSh::EncounterState required_state;
    std::chrono::milliseconds state_duration;
    bool full_battle_menu;
};
extern const DetectionType WILD_POKEMON;
extern const DetectionType YOUR_POKEMON;



// Used by detect_shiny_battle to detect dialog timing and 
// shiny sparkle animation to determine whether there is shiny pokemon
// when battle starts.
// See detect_shiny_battle() for more details.
class ShinyEncounterTracker : public VisualInferenceCallback{
    using EncounterDialogTracker = PokemonSwSh::EncounterDialogTracker;

public:
    ShinyEncounterTracker(
        Logger& logger, VideoOverlay& overlay,
        BattleType battle_type
    );

    const EncounterDialogTracker& dialog_tracker() const{ return m_dialog_tracker; }
    const ShinySparkleAggregator& sparkles_wild_overall() const{ return m_best_wild_overall; }
    const ShinySparkleAggregator& sparkles_wild_left() const{ return m_best_wild_left; }
    const ShinySparkleAggregator& sparkles_wild_right() const{ return m_best_wild_right; }
    const ShinySparkleAggregator& sparkles_own() const{ return m_best_own; }

    const WallClock& wild_animation_end_timestmap() const { return m_wild_animation_end_timestamp; }
    const WallClock& your_animation_start_timestamp() const { return m_your_animation_start_timestamp; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;

    ShinyType get_results() const;


private:
    Logger& m_logger;
//    VideoOverlay& m_overlay;

    BattleMenuWatcher m_battle_menu;

    BattleDialogDetector m_dialog_detector;
    EncounterDialogTracker m_dialog_tracker;
    WallClock m_wild_animation_end_timestamp;
    WallClock m_your_animation_start_timestamp;

    ImageFloatBox m_box_wild_left;
    ImageFloatBox m_box_wild_right;

    ShinySparkleSetBDSP m_sparkles_wild;
    ShinySparkleSetBDSP m_sparkles_own;
    ShinySparkleTracker m_sparkle_tracker_wild;
    ShinySparkleTracker m_sparkle_tracker_own;

    ShinySparkleAggregator m_best_wild_overall;
    ShinySparkleAggregator m_best_wild_left;
    ShinySparkleAggregator m_best_wild_right;
    ShinySparkleAggregator m_best_own;
};


// Called when battle starts to detect whether any pokemon in the battle is shiny.
// Store shiny results in `wild_result` and `your_result`.
// wild_result.shiny_type and your_result.shiny_type can be ShinyType::NOT_SHINY or 
// ShinyType::UNKNOWN_SHINY.
// `overall_threshold` is the threshold for determining wild shiny and your shiny.
// If there is wild shiny, `doubles_threshold` is the threshold to determine the shiniess
// of the left and right wild pokemon slot.
//
// Internally, we use the symbol alpha to denote how much shininess detected.
// The higher the alpha, the more likely the pokemon is shiny.
// `overall_threshold` and `doubles_threshold` are thresholds on alpha.
// Each detected shiny animation sparkles counts as 1.0 alpha.
// The function uses `ShinyEncounterTracker` to track the frame with the most detected
// sparkles, aka highest alpha.
// If the dialog timing is implies a shiny animation played, add 3.5 to the highest alpha.
// If the final value reaches `overall_threshold`, it is counted as a shiny detected.
// If the highest alpha detected on the cropped view of the left pokemon slot reaches 
// `doubles_threshold`, it is considered a shiny on left. Same to the right slot.
//
// The function also use a shiny sound detector to improve its detection on wild pokemon.
// When a shiny sound is detected, it adds 5.0 to the heighest overall alpha value.
void detect_shiny_battle(
    ProgramEnvironment& env,
    VideoStream& stream, CancellableScope& scope,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    EventNotificationOption& settings,
    const DetectionType& type,
    std::chrono::seconds timeout,
    bool use_shiny_sound = false
);








}
}
}
#endif
