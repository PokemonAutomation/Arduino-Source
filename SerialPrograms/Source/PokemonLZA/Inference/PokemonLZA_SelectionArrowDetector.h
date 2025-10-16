/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_SelectionArrowDetector_H
#define PokemonAutomation_PokemonLZA_SelectionArrowDetector_H

#include <optional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


enum class SelectionArrowType{
    RIGHT,
    DOWN,
};



class SelectionArrowDetector : public StaticScreenDetector{
public:
    SelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowType type,
        const ImageFloatBox& box
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class SelectionArrowWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const SelectionArrowType m_type;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class SelectionArrowWatcher : public DetectorToFinder<SelectionArrowDetector>{
public:
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowType type,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, overlay, type, box)
    {}
};





}
}
}
#endif
