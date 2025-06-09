/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleMenuDetector_H
#define PokemonAutomation_PokemonSwSh_BattleMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StandardBattleMenuDetector : public StaticScreenDetector{
public:
    StandardBattleMenuDetector(bool den, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    bool m_den;

    Color m_color;
    ImageFloatBox m_ball_left;
    ImageFloatBox m_ball_right;
    ImageFloatBox m_icon_fight;
    ImageFloatBox m_icon_pokemon;
    ImageFloatBox m_icon_bag;
    ImageFloatBox m_icon_run;
    ImageFloatBox m_text_fight;
    ImageFloatBox m_text_pokemon;
    ImageFloatBox m_text_bag;
    ImageFloatBox m_text_run;

//    ImageFloatBox m_status0;
    ImageFloatBox m_status1;
};


class StandardBattleMenuWatcher : public StandardBattleMenuDetector, public VisualInferenceCallback{
public:
    StandardBattleMenuWatcher(bool den, Color color = COLOR_RED);

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
