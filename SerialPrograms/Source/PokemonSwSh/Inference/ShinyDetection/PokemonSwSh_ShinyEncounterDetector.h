/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H
#define PokemonAutomation_PokemonSwSh_ShinyEncounterDetector_H

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "Pokemon/Pokemon_DataTypes.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogTracker.h"
#include "PokemonSwSh_ShinySparkleSet.h"

namespace PokemonAutomation{
    class CancellableScope;
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




class ShinyEncounterTracker : public VisualInferenceCallback{
public:
    ShinyEncounterTracker(
        Logger& logger, VideoOverlay& overlay,
        const ShinyDetectionBattle& battle_settings
    );

    const EncounterDialogTracker& dialog_tracker() const{ return m_dialog_tracker; }
    const ShinySparkleAggregator& sparkles_wild() const{ return m_best_wild; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;

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
    double& alpha,
    Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    const EncounterDialogTracker& dialog_tracker,
    const ShinySparkleAggregator& sparkles
);




ShinyDetectionResult detect_shiny_battle(
    VideoStream& stream, CancellableScope& scope,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout
);




}
}
}
#endif
