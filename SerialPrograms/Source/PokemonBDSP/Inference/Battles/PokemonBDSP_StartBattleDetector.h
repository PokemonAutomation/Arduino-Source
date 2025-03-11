/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StartBattleDetector_H
#define PokemonAutomation_PokemonBDSP_StartBattleDetector_H

#include <atomic>
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{



class StartBattleDetector : public VisualInferenceCallback{
public:
    StartBattleDetector(VideoOverlay& overlay);

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

private:
    ImageFloatBox m_screen_box;
    BattleDialogDetector m_dialog;
};


class StartBattleMenuOverlapDetector : public VisualInferenceCallback{
public:
    StartBattleMenuOverlapDetector(VideoOverlay& overlay);

    bool detected() const{ return m_battle_detected.load(std::memory_order_acquire); }
    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

private:
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    std::atomic<bool> m_battle_detected;
};







}
}
}
#endif

