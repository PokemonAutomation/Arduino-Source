/*  Door Entry Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DoorEntryDetector_H
#define PokemonAutomation_PokemonLZA_DoorEntryDetector_H

#include <chrono>
#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
class UnitTestDatabase;
namespace NintendoSwitch{
namespace PokemonLZA{

class DoorEntryDetector : public StaticScreenDetector{
public:
    DoorEntryDetector(
        Color color, VideoOverlay* overlay = nullptr,
        const ImageFloatBox& box = {0.2, 0.15, 0.65, 0.65}
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual void reset_state() override;

    const ImageFloatBox& last_detected() const{ return m_last_detected; }

private:
    Color m_color;
    VideoOverlay* m_overlay;
    ImageFloatBox m_box;
    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

class DoorEntryWatcher : public DetectorToFinder<DoorEntryDetector>{
public:
    DoorEntryWatcher(
        Color color, VideoOverlay* overlay = nullptr,
        const ImageFloatBox& box = {0.2, 0.15, 0.65, 0.65},
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(200)
    )
        : DetectorToFinder("DoorEntryWatcher", hold_duration, color, overlay, box)
    {}
};

void add_tests_DoorEntryDetector(UnitTestDatabase& database);

}
}
}
#endif
