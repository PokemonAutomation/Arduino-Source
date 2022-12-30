/*  Sandwich Hand Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichHandLocator_H
#define PokemonAutomation_PokemonSV_SandwichHandLocator_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class SandwichHandType {
    FREE,
    GRABBING,
};

// Type enum to string
std::string SANDWOCH_HAND_TYPE_NAMES(SandwichHandType type);

// Detect the hand used to make sandwich in the sandwich minigame.
class SandwichHandLocator{
public:
    using HandType = SandwichHandType;

    SandwichHandLocator(HandType hand_type, const ImageFloatBox& box, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    
    // return detected hand cetner location on screen, (x, y) where
    // x: [0, 1), 0 left-most, 1 right-most, y: [0, 1), 0 top-most 1 bottom-most
    // If hand not detected, return (-1, -1).
    std::pair<double, double> detect(const ImageViewRGB32& screen) const;

    void change_box(const ImageFloatBox& new_box) { m_box = new_box; }

private:
    HandType m_type;
    ImageFloatBox m_box;
    Color m_color;
};

class SandwichHandWatcher : public VisualInferenceCallback{
public:
    using HandType = SandwichHandType;

    SandwichHandWatcher(HandType hand_type, const ImageFloatBox& box, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::pair<double, double>& location() const { return m_location; }

    void change_box(const ImageFloatBox& new_box) { m_locator.change_box(new_box); }

private:
    SandwichHandLocator m_locator;
    std::pair<double, double> m_location;
};



}
}
}
#endif
