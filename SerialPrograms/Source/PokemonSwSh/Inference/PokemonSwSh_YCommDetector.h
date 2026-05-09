/*  Y-Comm Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_YCommDetector_H
#define PokemonAutomation_PokemonSwSh_YCommDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{

class Logger;

namespace NintendoSwitch{
namespace PokemonSwSh{


class YCommMenuDetector : public VisualInferenceCallback{
public:
    YCommMenuDetector(bool is_on);

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;

private:
    bool m_is_on;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom;
};

// Detect the blue Y letter as the Y Comm symbol in the lower left corner of the screen on the
// overworld, when Y Comm is active.
class YCommIconDetector : public StaticScreenDetector{
public:
    // If `is_on` is true, `process_frame()` returns true if it finds the Y letter.
    // Otherwise, `process_frame()` returns true if the Y letter is not found.
    YCommIconDetector(Color color, bool is_on);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_icon;
    ImageFloatBox m_left;
    ImageFloatBox m_right;

    bool m_is_on;
};
class YCommIconWatcher : public DetectorToFinder<YCommIconDetector>{
public:
    YCommIconWatcher(Color color = COLOR_RED, bool is_on = true)
         : DetectorToFinder("YCommIconWatcher", std::chrono::milliseconds(250), color, is_on)
    {}
};





}
}
}
#endif
