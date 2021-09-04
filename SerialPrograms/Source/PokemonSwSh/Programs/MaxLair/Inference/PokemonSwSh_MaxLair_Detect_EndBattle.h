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


class PokemonCaughtMenuDetector : public VisualInferenceCallbackWithCommandStop{
public:
    PokemonCaughtMenuDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    InferenceBoxScope m_top_white;
    InferenceBoxScope m_caught_left;
    InferenceBoxScope m_caught_right;
    InferenceBoxScope m_middle_pink;
    InferenceBoxScope m_bottom_white;
    InferenceBoxScope m_bottom_black;
    InferenceBoxScope m_bottom_options;


};



}
}
}
}
#endif
