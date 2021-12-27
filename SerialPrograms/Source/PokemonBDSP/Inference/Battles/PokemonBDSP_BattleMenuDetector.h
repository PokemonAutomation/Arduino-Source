/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleMenuDetector_H
#define PokemonAutomation_PokemonBDSP_BattleMenuDetector_H

#include <memory>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BattleType{
    STARTER,
    WILD,
    TRAINER,
};


class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(BattleType battle_type, QColor color = Qt::red);

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool detect(const QImage& screen) const override;

private:
    QColor m_color;
    BattleType m_battle_type;
    ImageFloatBox m_left0_status;
    ImageFloatBox m_left1_status;
    ImageFloatBox m_right_status;
    ImageFloatBox m_ball_left;
    ImageFloatBox m_ball_right;
    ImageFloatBox m_menu_battle;
    ImageFloatBox m_menu_pokemon;
    ImageFloatBox m_menu_bag;
    ImageFloatBox m_menu_run;
};


class BattleMenuWatcher : public BattleMenuDetector, public VisualInferenceCallback{
public:
    using BattleMenuDetector::BattleMenuDetector;

    virtual void make_overlays(OverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    size_t m_trigger_count = 0;
};



}
}
}
#endif
