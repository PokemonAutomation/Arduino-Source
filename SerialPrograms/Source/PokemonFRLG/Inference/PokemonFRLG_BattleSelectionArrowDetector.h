/*  Battle Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleSelectionArrowDetector_H
#define PokemonAutomation_PokemonFRLG_BattleSelectionArrowDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


// The order of these enums matches the in-game layout (row-major, top-left to bottom-right).
const int BATTLE_MENU_OPTION_COUNT = 4;
enum class BattleMenuOption{
    FIGHT,
    BAG,
    POKEMON,
    RUN,
};

const int SAFARI_BATTLE_MENU_OPTION_COUNT = 4;
enum class SafariBattleMenuOption{
    BALL,
    BAIT,
    ROCK,
    RUN,
};

enum class BattleConfirmationOption{
    YES,
    NO,
};

class BattleSelectionArrowDetector : public StaticScreenDetector{
public:
    BattleSelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );
    BattleSelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        BattleMenuOption option
    );
    BattleSelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        SafariBattleMenuOption option
    );
    BattleSelectionArrowDetector(
        Color color,
        VideoOverlay* overlay
    );
    BattleSelectionArrowDetector(
        Color color,
        VideoOverlay* overlay,
        BattleConfirmationOption option
    );

    static ImageFloatBox box_for_option(BattleMenuOption option);
    static ImageFloatBox box_for_option(SafariBattleMenuOption option);
    static ImageFloatBox box_for_option(BattleConfirmationOption option);

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BattleSelectionArrowWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class BattleSelectionArrowWatcher : public DetectorToFinder<BattleSelectionArrowDetector>{
public:
    BattleSelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("BattleSelectionArrowWatcher", hold_duration, color, overlay, box)
    {}
    BattleSelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        BattleMenuOption option,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("BattleSelectionArrowWatcher", hold_duration, color, overlay, box_for_option(option))
    {}
    BattleSelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        SafariBattleMenuOption option,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("BattleSelectionArrowWatcher", hold_duration, color, overlay, box_for_option(option))
    {}
    BattleSelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        BattleConfirmationOption option,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("BattleSelectionArrowWatcher", hold_duration, color, overlay, box_for_option(option))
    {}
    BattleSelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("BattleSelectionArrowWatcher", hold_duration, color, overlay, ImageFloatBox(0.768, 0.868, 0.212, 0.159))
    {}
};


}
}
}
#endif
