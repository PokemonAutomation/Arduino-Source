/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StartBattleDetector_H
#define PokemonAutomation_PokemonBDSP_StartBattleDetector_H

#include <functional>
#include <chrono>
#include <thread>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonBDSP{



class StartBattleDetector : public VisualInferenceCallback{
public:
    StartBattleDetector(VideoOverlay& overlay);

    bool detect(const QImage& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;

private:
    ImageFloatBox m_screen_box;
    BattleDialogDetector m_dialog;
};


class StartBattleMenuOverlapDetector : public VisualInferenceCallback{
public:
    StartBattleMenuOverlapDetector(VideoOverlay& overlay);

    bool detected() const{ return m_battle_detected.load(std::memory_order_acquire); }
    bool detect(const QImage& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const QImage& frame, WallClock timestamp) override final;

private:
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    std::atomic<bool> m_battle_detected;
};







}
}
}
#endif

