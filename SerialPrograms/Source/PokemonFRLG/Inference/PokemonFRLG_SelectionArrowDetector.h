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

// The order of these enums should be the same as the order of options in the game menu, from top to bottom, for ease of use with loops.
const int SAFARI_START_MENU_OPTION_COUNT = 7;
enum class SelectionArrowPositionSafariMenu {
    RETIRE,
    POKEDEX,
    POKEMON,
    BAG,
    TRAINER,
    OPTION,
    EXIT
};

const int START_MENU_OPTION_COUNT = 7;
enum class SelectionArrowPositionStartMenu{
    POKEDEX,
    POKEMON,
    BAG,
    TRAINER,
    SAVE,
    OPTION,
    EXIT
};

enum class SelectionArrowPositionConfirmationMenu {
    YES,
    NO
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
        SelectionArrowPositionStartMenu position
    );

    SelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPositionSafariMenu position
    );

    SelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPositionConfirmationMenu position
    );

    static ImageFloatBox arrow_box_for_position(SelectionArrowPositionStartMenu position);

    static ImageFloatBox arrow_box_for_position(SelectionArrowPositionSafariMenu position);

    static ImageFloatBox arrow_box_for_position(SelectionArrowPositionConfirmationMenu position);

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // This is not const so that detectors can save/cache state.
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
        SelectionArrowPositionStartMenu position,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, overlay, arrow_box_for_position(position))
    {
    }
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPositionSafariMenu position,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, overlay, arrow_box_for_position(position))
    {
    }
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        SelectionArrowPositionConfirmationMenu position,
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