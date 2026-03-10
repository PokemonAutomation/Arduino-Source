/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_SelectionArrowDetector_H
#define PokemonAutomation_PokemonFRLG_SelectionArrowDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

enum class SelectionArrowPosition{
    START_MENU_POKEDEX,
    START_MENU_POKEMON,
    START_MENU_BAG,
    START_MENU_TRAINER,
    START_MENU_SAVE,
    START_MENU_OPTION,
    START_MENU_EXIT,
    CHOICE_MENU_YES,
    CHOICE_MENU_NO
};

class SelectionArrowDetector : public StaticScreenDetector{
public:
    SelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );

    SelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPosition position
    );

    static ImageFloatBox arrow_box_for_position(SelectionArrowPosition position);

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class SelectionArrowWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class SelectionArrowWatcher : public DetectorToFinder<SelectionArrowDetector>{
public:
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, overlay, box)
    {}
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPosition position,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, overlay, arrow_box_for_position(position))
    {
    }
};


}
}
}

#endif