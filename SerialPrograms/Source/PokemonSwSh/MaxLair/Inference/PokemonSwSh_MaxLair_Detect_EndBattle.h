/*  Max Lair Detect End Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_EndBattle_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_EndBattle_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class VideoOverlay;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class PokemonCaughtMenuDetector : public VisualInferenceCallback{
public:
    PokemonCaughtMenuDetector();

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_top_white;
    ImageFloatBox m_caught_left;
    ImageFloatBox m_caught_right;
    ImageFloatBox m_middle_pink;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_bottom_black;
    ImageFloatBox m_bottom_options;

};




size_t count_catches(VideoOverlay& overlay, const ImageViewRGB32& screen);




}
}
}
}
#endif
