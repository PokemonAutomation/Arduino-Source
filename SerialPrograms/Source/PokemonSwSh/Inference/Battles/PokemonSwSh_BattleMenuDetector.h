/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleMenuDetector_H
#define PokemonAutomation_PokemonSwSh_BattleMenuDetector_H

#include <memory>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardBattleMenuDetector : public VisualInferenceCallbackWithCommandStop{
public:
    StandardBattleMenuDetector(VideoOverlay& overlay, bool den);

    bool detect(const QImage& screen) const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    std::unique_ptr<InferenceBoxScope> m_ball_left;
    std::unique_ptr<InferenceBoxScope> m_ball_right;
    InferenceBoxScope m_icon_fight;
    InferenceBoxScope m_icon_pokemon;
    InferenceBoxScope m_icon_bag;
    InferenceBoxScope m_icon_run;
    InferenceBoxScope m_text_fight;
    InferenceBoxScope m_text_pokemon;
    InferenceBoxScope m_text_bag;
    InferenceBoxScope m_text_run;
};



}
}
}
#endif
