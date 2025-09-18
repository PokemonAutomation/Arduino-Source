/*  Max Lair Detect Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_Entrance_H

#include <memory>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


class EntranceDetector : public VisualInferenceCallback{
public:
    EntranceDetector(const ImageViewRGB32& entrance_screen);

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    ImageFloatBox m_box0;
    ImageViewRGB32 m_watch_box;
    std::shared_ptr<const ImageRGB32> m_entrance_screen;
};



}
}
}
}
#endif
