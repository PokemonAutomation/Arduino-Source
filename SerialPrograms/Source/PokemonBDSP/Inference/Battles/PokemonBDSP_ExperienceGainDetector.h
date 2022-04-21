/*  Battle Won Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleWonDetector_H
#define PokemonAutomation_PokemonBDSP_BattleWonDetector_H

#include "Common/Cpp/FixedLimitVector.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// Detect the screen showing the party pokemon are about to gain experience.
// This is useful to detect whether you have caught or defeated a wild pokemon.
// The experience screen still shows even if all party pokemon are max leveled and have max EVs.
class ExperienceGainDetector : public StaticScreenDetector{
public:
    ~ExperienceGainDetector();
    ExperienceGainDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    Color m_color;
    ShortDialogDetector m_dialog;
    ImageFloatBox m_middle_column;
    ImageFloatBox m_left_column;
    ImageFloatBox m_lower_left_region;
};

// Watch for the screen that the party pokemon are about to gain experience.
// The experience screen still shows even if all party pokemon are max leveled and have max EVs.
class ExperienceGainWatcher : public ExperienceGainDetector, public VisualInferenceCallback{
public:
    ExperienceGainWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    // Return true when the screen that the party pokemon are about to gain experience is found.
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;
};


}
}
}
#endif
