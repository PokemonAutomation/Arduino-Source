/*  Tera Rewards Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRewardsDetector_H
#define PokemonAutomation_PokemonSV_TeraRewardsDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
namespace NintendoSwitch{
namespace PokemonSV{



class SparklyItemDetector : public VisualInferenceCallback{
    static const size_t ITEMS = 8;

public:
    SparklyItemDetector(Color color = COLOR_RED);

    static size_t count_sparkly_items(VideoStream& stream, CancellableScope& scope);

    size_t sparkly_items() const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Color m_color;
    ImageFloatBox m_boxes[ITEMS];
    ImageRGB32 m_last[ITEMS];
    bool m_sparkly[ITEMS];
    WallClock m_start_time;
};





}
}
}
#endif
