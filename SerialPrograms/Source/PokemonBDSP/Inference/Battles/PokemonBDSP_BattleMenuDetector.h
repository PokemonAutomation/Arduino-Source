/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleMenuDetector_H
#define PokemonAutomation_PokemonBDSP_BattleMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class BattleType{
    STARTER,
    STANDARD,
    GREAT_MARSH,
    TRAINER,
};


class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(BattleType battle_type, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    bool is_battle_button(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    BattleType m_battle_type;
    ImageFloatBox m_left0_status;
    ImageFloatBox m_left1_status;
    ImageFloatBox m_right_status;
    ImageFloatBox m_opponent_left;
    ImageFloatBox m_opponent_right;
    ImageFloatBox m_ball_left;
    ImageFloatBox m_ball_right;
    ImageFloatBox m_menu_battle;
    ImageFloatBox m_menu_pokemon;
    ImageFloatBox m_menu_bag;
    ImageFloatBox m_menu_run;
};


class BattleMenuWatcher : public BattleMenuDetector, public VisualInferenceCallback{
public:
    BattleMenuWatcher(BattleType battle_type, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;
    using VisualInferenceCallback::process_frame;


private:
    size_t m_trigger_count = 0;
};



}
}
}
#endif
