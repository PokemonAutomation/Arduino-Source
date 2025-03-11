/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StartBattleDetector_H
#define PokemonAutomation_PokemonSwSh_StartBattleDetector_H

#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class StartBattleWatcher : public VisualInferenceCallback{
public:
    StartBattleWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

private:
    Color m_color;
    ImageFloatBox m_screen_box;
    BattleDialogDetector m_dialog;
};







}
}
}
#endif

