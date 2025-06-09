/*  Poke Portal Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokePortalDetector_H
#define PokemonAutomation_PokemonSV_PokePortalDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class PokePortalDetector : public StaticScreenDetector{
public:
    PokePortalDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Read where the cursor is. Returns -1 on failure.
    int detect_location(const ImageViewRGB32& screen);

    //  While sitting on the menu, move the cursor to the desired slot.
    //  Returns true if success.
    bool move_cursor(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        int row
    );


private:
    Color m_color;
    ImageFloatBox m_bottom;
    GradientArrowDetector m_arrow_union;
    GradientArrowDetector m_arrow_tera;
    GradientArrowDetector m_arrow_bottom;
};
class PokePortalWatcher : public DetectorToFinder<PokePortalDetector>{
public:
    PokePortalWatcher(Color color = COLOR_RED)
         : DetectorToFinder("PokePortalWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
