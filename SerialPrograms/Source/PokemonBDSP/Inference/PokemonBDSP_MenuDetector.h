/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MenuDetector_H
#define PokemonAutomation_PokemonBDSP_MenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MenuDetector : public StaticScreenDetector{
public:
    //  The menu is recognised by the white gaps between its entries. BDSP tags an
    //  entry with a red "NEW" banner the first time its contents change -- on the
    //  POKEMON entry once a starter is in hand, for instance -- and that banner sits
    //  directly over the second gap. is_white() allows a summed stddev of only 10 and
    //  the banner's edge takes it to 15, so the whole detector goes blind while the
    //  tag is present.
    //
    //  "skip_new_banner" starts the gaps below the banner instead. It costs the top
    //  ~12% of the panel, which is white either way, so it only ever makes detection
    //  easier -- it cannot turn a screen the default accepts into one it rejects.
    //
    //  Off by default so existing callers are unchanged. Any BDSP program that opens
    //  the menu while a NEW tag is showing wants it on.
    MenuDetector(Color color = COLOR_RED, bool skip_new_banner = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_line0;
    ImageFloatBox m_line1;
    ImageFloatBox m_line2;
    ImageFloatBox m_line3;
    ImageFloatBox m_line4;
    ImageFloatBox m_cross;
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
