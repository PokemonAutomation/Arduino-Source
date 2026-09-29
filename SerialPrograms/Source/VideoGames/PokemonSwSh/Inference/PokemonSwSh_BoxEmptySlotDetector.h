/*  Box Empty Slot Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxEmptySlotDetector_H
#define PokemonAutomation_PokemonSwSh_BoxEmptySlotDetector_H

#include "Common/Cpp/Color.h"
#include "CommonTools/VisualDetector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSwSh_BoxEggDetector.h"
// #include "CommonTools/ImageMatch/ExactImageDictionaryMatcher.h"
// #include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

// Determine if given slot is empty

class BoxEmptySlotDetector : public StaticScreenDetector{
public:
    BoxEmptySlotDetector(SlotLocation side, uint8_t row, uint8_t column, Color color = COLOR_BLUE);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;


private:
    Color m_color;
    ImageFloatBox m_box;
};



}
}
}
#endif
