/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleMenuDetector_H
#define PokemonAutomation_PokemonBDSP_BattleMenuDetector_H

#include <memory>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BattleType{
    STARTER,
    WILD,
    TRAINER,
};


class BattleMenuDetector : public VisualInferenceCallback{
public:
    BattleMenuDetector(BattleType battle_type);

    bool detect(const QImage& screen) const;

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    BattleType m_battle_type;
    ImageFloatBox m_left_status;
    ImageFloatBox m_right_status;
    ImageFloatBox m_ball_left;
    ImageFloatBox m_ball_right;
    ImageFloatBox m_menu_battle;
    ImageFloatBox m_menu_pokemon;
    ImageFloatBox m_menu_bag;
    ImageFloatBox m_menu_run;

    size_t m_trigger_count = 0;
};



}
}
}
#endif
