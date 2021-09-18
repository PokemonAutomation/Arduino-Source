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


class StandardBattleMenuDetector : public VisualInferenceCallback{
public:
    StandardBattleMenuDetector(bool den);

    bool detect(const QImage& screen) const;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    std::unique_ptr<ImageFloatBox> m_ball_left;
    std::unique_ptr<ImageFloatBox> m_ball_right;
    ImageFloatBox m_icon_fight;
    ImageFloatBox m_icon_pokemon;
    ImageFloatBox m_icon_bag;
    ImageFloatBox m_icon_run;
    ImageFloatBox m_text_fight;
    ImageFloatBox m_text_pokemon;
    ImageFloatBox m_text_bag;
    ImageFloatBox m_text_run;
};



}
}
}
#endif
