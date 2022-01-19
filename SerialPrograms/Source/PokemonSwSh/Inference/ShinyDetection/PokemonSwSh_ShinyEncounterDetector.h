/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "Pokemon/Options/Pokemon_EncounterBotOptions.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"
#include "PokemonSwSh_ShinySparkleSet.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


struct ShinyDetectionBattle{
    bool den;
    ImageFloatBox detection_box;
    std::chrono::milliseconds dialog_delay_when_shiny;
};
extern const ShinyDetectionBattle SHINY_BATTLE_REGULAR;
extern const ShinyDetectionBattle SHINY_BATTLE_RAID;



ShinyDetectionResult detect_shiny_battle(
    ProgramEnvironment& env, Logger& logger,
    VideoFeed& feed, VideoOverlay& overlay,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold = 2.0
);




class ShinyEncounterTracker : public VisualInferenceCallback{
public:
    ShinyEncounterTracker(
        Logger& logger, VideoOverlay& overlay,
        const ShinyDetectionBattle& battle_settings
    );

    const EncounterDialogTracker& dialog_timer() const{ return m_dialog_tracker; }
    const ShinySparkleAggregator& sparkles_wild() const{ return m_best_wild; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

    ShinyType get_results() const;


private:
    ShinyDetectionBattle m_battle_settings;

    Logger& m_logger;
//    VideoOverlay& m_overlay;

    StandardBattleMenuWatcher m_battle_menu;

    BattleDialogDetector m_dialog_detector;
    EncounterDialogTracker m_dialog_tracker;

    ShinySparkleSetSwSh m_sparkles;
    ShinySparkleTracker m_sparkle_tracker;

    ShinySparkleAggregator m_best_wild;
};

ShinyType determine_shiny_status(
    Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    const EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles,
    double detection_threshold = 2.0
);




}
}
}
#endif
