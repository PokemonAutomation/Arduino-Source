/*  Box Egg Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxEggDetector_H
#define PokemonAutomation_PokemonSV_BoxEggDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

// Detect if the currently selected pokemon is an egg or not, assuming the current
// selected slot is not empty and the box view is stats or judege mode.
class BoxCurrentEggDetector : public StaticScreenDetector{
public:
    BoxCurrentEggDetector(Color color = COLOR_BLUE);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};


// Detect if a slot in the box system is an egg.
class BoxEggDetector : public StaticScreenDetector{
public:
    BoxEggDetector(BoxCursorLocation side, uint8_t row, uint8_t col, Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

class BoxEggWatcher : public DetectorToFinder<BoxEggDetector>{
public:
    BoxEggWatcher(BoxCursorLocation side, uint8_t row, uint8_t col, FinderType finder_type = FinderType::PRESENT, Color color = COLOR_YELLOW)
         : DetectorToFinder("BoxEggWatcher", finder_type, std::chrono::milliseconds(100), side, row, col, color)
    {}
};

// Detect eggs in a party column (five slots after the party lead). Used for egg hatching.
// It also detects empty spaces and non-empty spaces, so that we know whether we have non-egg pokemon in the party.
class BoxEggPartyColumnWatcher : public VisualInferenceCallback{
public:
    BoxEggPartyColumnWatcher(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    
    // Return true when the watcher is sure that each of the five slots is either egg, non-egg pokemon or empty.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    uint8_t num_eggs_found() const;
    
    uint8_t num_non_egg_pokemon_found() const;

private:
    FixedLimitVector<BoxEggWatcher> m_egg_watchers;
    FixedLimitVector<BoxEmptySlotWatcher> m_empty_watchers;
};



}
}
}
#endif
