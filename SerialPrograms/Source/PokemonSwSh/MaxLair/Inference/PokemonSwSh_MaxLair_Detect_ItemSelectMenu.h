/*  Max Lair Detect Item Select Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_ItemSelectMenu_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class ItemSelectDetector : public VisualInferenceCallback{
public:
    ItemSelectDetector(bool stop_no_detect);

    bool detect(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    bool m_stop_on_no_detect;
    ImageFloatBox m_bottom_main;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_blue;
};


}
}
}
}
#endif
