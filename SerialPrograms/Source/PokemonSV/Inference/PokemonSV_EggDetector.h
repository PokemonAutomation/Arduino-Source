/*  Egg Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggDetector_H
#define PokemonAutomation_PokemonSV_EggDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// Detect the player character is in a picnic
class EggDetector : public StaticScreenDetector{
public:
    EggDetector(BoxCursorLocation side, uint8_t row, uint8_t col, Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

class EggWatcher : public DetectorToFinder<EggDetector>{
public:
    EggWatcher(BoxCursorLocation side, uint8_t row, uint8_t col, FinderType finder_type = FinderType::PRESENT, Color color = COLOR_YELLOW)
         : DetectorToFinder("EggWatcher", finder_type, std::chrono::milliseconds(100), side, row, col, color)
    {}
};

// Detect eggs in a party column (five slots after the party lead). Used for egg hatching.
// It also detects empty spaces and non-empty spaces, so that we know whether we have non-egg pokemon in the party.
class EggPartyColumnWatcher : public VisualInferenceCallback{
public:
    EggPartyColumnWatcher(Color color = COLOR_YELLOW);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    
    // Return true when the watcher is sure that each of the five slots is either egg, non-egg pokemon or empty.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    uint8_t num_eggs_found() const;
    
    uint8_t num_non_egg_pokemon_found() const;

private:
    FixedLimitVector<EggWatcher> m_egg_watchers;
    FixedLimitVector<BoxEmptySlotWatcher> m_empty_watchers;
};



}
}
}
#endif
