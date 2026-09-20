/*  Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_ShinyDetector_H
#define PokemonAutomation_PokemonHome_ShinyDetector_H

#include <optional>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class ShinyDetector : public StaticScreenDetector{
public:
    ShinyDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    VideoOverlay* m_overlay;
    ImageFloatBox m_box;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

void add_tests_ShinyDetector(UnitTestDatabase& database);


}
}
}
#endif
