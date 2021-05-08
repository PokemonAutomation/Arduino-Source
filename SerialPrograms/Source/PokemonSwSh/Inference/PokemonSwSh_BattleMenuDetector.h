/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Drop a wishing piece and determine if it is red or purple.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleMenuDetector_H
#define PokemonAutomation_PokemonSwSh_BattleMenuDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class StandardBattleMenuDetector : public VisualInferenceCallbackWithCommandStop{
public:
    StandardBattleMenuDetector(VideoFeed& feed);

    bool detect(const QImage& image) const;

    virtual bool on_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
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
