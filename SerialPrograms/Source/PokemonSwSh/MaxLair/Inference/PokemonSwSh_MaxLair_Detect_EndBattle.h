/*  Max Lair Detect End Battle
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_EndBattle_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_EndBattle_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PokemonCaughtMenuDetector : public VisualInferenceCallback{
public:
    PokemonCaughtMenuDetector();

    bool detect(const QImage& screen);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_top_white;
    ImageFloatBox m_caught_left;
    ImageFloatBox m_caught_right;
    ImageFloatBox m_middle_pink;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_bottom_black;
    ImageFloatBox m_bottom_options;

};




size_t count_catches(VideoOverlay& overlay, const QImage& screen);




}
}
}
}
#endif
