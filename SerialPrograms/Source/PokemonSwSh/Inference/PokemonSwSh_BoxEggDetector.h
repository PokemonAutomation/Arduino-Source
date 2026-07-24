/*  Box Egg Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxEggDetector_H
#define PokemonAutomation_PokemonSwSh_BoxEggDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
#include "CommonTools/VisualDetector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class SlotLocation{
    PARTY,      // player's party
    BOX, //  one of the 5 x 6 slots in the box
};

class BoxEggDetector : public StaticScreenDetector{
public:
    BoxEggDetector(SlotLocation side, uint8_t row, double min_euclidean_distance = 100, Color color = COLOR_BLUE);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const;

private:
    double m_min_euclidean_distance_squared;
    Color m_color;
    ImageFloatBox m_box;
};



}
}
}
#endif
