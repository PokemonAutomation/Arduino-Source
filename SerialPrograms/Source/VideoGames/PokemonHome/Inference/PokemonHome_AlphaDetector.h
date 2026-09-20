/*  Alpha Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_AlphaDetector_H
#define PokemonAutomation_PokemonHome_AlphaDetector_H

#include <optional>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class AlphaDetector : public StaticScreenDetector{
public:
    AlphaDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    VideoOverlay* m_overlay;
    ImageFloatBox m_box;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

void add_tests_AlphaDetector(UnitTestDatabase& database);


}
}
}
#endif
