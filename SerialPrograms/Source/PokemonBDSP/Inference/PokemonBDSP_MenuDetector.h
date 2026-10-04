/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MenuDetector_H
#define PokemonAutomation_PokemonBDSP_MenuDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// Screenshot tests accept no controller context. non_menu_*.png are negative cases.
std::string menu_detector_test_folder();
bool run_menu_detector_tests(Logger& logger, const std::string& folder);
void add_tests_MenuDetector(UnitTestDatabase& database);
int test_MenuDetector(const ImageViewRGB32& image, bool expected);

class MenuDetector : public StaticScreenDetector{
public:
    MenuDetector(Color color = COLOR_RED, bool skip_new_banner = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    bool detect_menu_chrome(const ImageViewRGB32& screen) const;

    Color m_color;
    ImageFloatBox m_line0;
    ImageFloatBox m_line1;
    ImageFloatBox m_line2;
    ImageFloatBox m_line3;
    ImageFloatBox m_line4;
    ImageFloatBox m_cross;
    ImageFloatBox m_panel_top;
    ImageFloatBox m_panel_left;
    ImageFloatBox m_panel_right;
    ImageFloatBox m_footer;
    ImageFloatBox m_confirm_button;
    ImageFloatBox m_save_button;
    ImageFloatBox m_close_button;
    ImageFloatBox m_save_heading;
    ImageFloatBox m_save_background;
    ImageFloatBox m_save_shortcut;
};


class MenuWatcher : public MenuDetector, public VisualInferenceCallback{
public:
    MenuWatcher(Color color = COLOR_RED, bool skip_new_banner = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};




}
}
}
#endif
