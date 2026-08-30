/*  Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_SelectionDetector_H
#define PokemonAutomation_PokemonRSE_SelectionDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


constexpr double SELECTION_ARROW_BOX_WIDTH = 0.039;
constexpr double SELECTION_ARROW_BOX_HEIGHT = 0.10;
constexpr double SELECTION_BORDER_BOX_WIDTH = 0.02;
constexpr double SELECTION_BORDER_BOX_HEIGHT = 0.084;

constexpr double SELECTION_SLOT_OFFSETS[] = {
    0.0,    // slot 0
    0.10,   // slot 1
    0.20,   // slot 2
    0.30,   // slot 3
    0.40,   // slot 4
    0.50,   // slot 5
    0.60,   // slot 6
    0.70,   // slot 7
};

enum class SelectionIndicator {
    ARROW,
    BORDER,
};

struct SlotCandidate {
    double x;
    double y;
    SelectionIndicator indicator;
};


// international Emerald, japanese Ruby/Sapphire/Emerald
class SelectionArrowDetector : public StaticScreenDetector{
public:
    SelectionArrowDetector(
        Color color,
        const ImageFloatBox& box
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const Color m_color;
    const ImageFloatBox m_box;
};
class SelectionArrowWatcher : public DetectorToFinder<SelectionArrowDetector>{
public:
    SelectionArrowWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("SelectionArrowWatcher", hold_duration, color, box)
    {}
};


// international Ruby/Sapphire
class SelectionBorderDetector : public StaticScreenDetector {
public:
    SelectionBorderDetector(
        Color color,
        const ImageFloatBox& box
    );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    const ImageFloatBox m_box;
};
class SelectionBorderWatcher : public DetectorToFinder<SelectionBorderDetector> {
public:
    SelectionBorderWatcher(
        Color color,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("SelectionBorderWatcher", hold_duration, color, box)
    {}
};


// checks for one selection slot
// In most cases uses at least one arrow and one border detector
class SelectionSlotDetector : public StaticScreenDetector {
public: 
    SelectionSlotDetector(
        Color color,
        const std::vector<SlotCandidate>& candidates
        );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    std::vector<std::unique_ptr<StaticScreenDetector>> m_detectors;
};
class SelectionWatcher : public DetectorToFinder<SelectionSlotDetector> {
public:
    SelectionWatcher(
        Color color,
        VideoOverlay* overlay,
        const std::vector<SlotCandidate>& candidates,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("SelectionWatcher", hold_duration, color, candidates)
    {
    }
};


}
}
}

#endif