/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FishingDetector_H
#define PokemonAutomation_PokemonSwSh_FishingDetector_H

#include <chrono>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FishingMissDetector : public VisualInferenceCallback{
public:
    FishingMissDetector();

    bool detect(const QImage& frame);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    ImageFloatBox m_hook_box;
    ImageFloatBox m_miss_box;
};

class FishingHookDetector : public VisualInferenceCallback{
public:
    FishingHookDetector(VideoOverlay& overlay);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

private:
    VideoOverlay& m_overlay;
    ImageFloatBox m_hook_box;
    std::deque<InferenceBoxScope> m_marks;
};



}
}
}
#endif
