/*  Pokemon Home Page View Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_PageViewDetector_H
#define PokemonAutomation_PokemonHome_PageViewDetector_H

#include "Common/Cpp/Color.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonHome_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class PageViewDetector : public StaticScreenDetector{
public:
    explicit PageViewDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    virtual void reset_state() override{ m_button_plus_detector.reset_state(); }

private:
    Color m_color;
    ButtonDetector m_button_plus_detector;
};

void add_tests_PageViewDetector(UnitTestDatabase& database);

}
}
}
#endif
