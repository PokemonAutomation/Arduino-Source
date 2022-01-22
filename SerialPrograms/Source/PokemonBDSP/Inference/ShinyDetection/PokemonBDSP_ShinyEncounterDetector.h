/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonBDSP_ShinyEncounterDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"
#include "PokemonBDSP_ShinySparkleSet.h"

namespace PokemonAutomation{
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


void detect_shiny_battle(
    ProgramEnvironment& env, Logger& logger,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    VideoFeed& feed, VideoOverlay& overlay,
    const DetectionType& type,
    std::chrono::seconds timeout,
    double overall_threshold = 5.0,
    double doubles_threshold = 3.0
);





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

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

    ShinyType get_results() const;


private:
    BattleType m_battle_type;

    Logger& m_logger;
//    VideoOverlay& m_overlay;

    BattleMenuWatcher m_battle_menu;

    BattleDialogDetector m_dialog_detector;
    EncounterDialogTracker m_dialog_tracker;

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

void determine_shiny_status(
    Logger& logger,
    DoublesShinyDetection& wild_result,
    ShinyDetectionResult& your_result,
    const PokemonSwSh::EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles_wild_overall,
    const ShinySparkleAggregator& sparkles_wild_left,
    const ShinySparkleAggregator& sparkles_wild_right,
    const ShinySparkleAggregator& sparkles_own,
    double overall_threshold = 5.0,
    double doubles_threshold = 3.0
);








}
}
}
#endif
