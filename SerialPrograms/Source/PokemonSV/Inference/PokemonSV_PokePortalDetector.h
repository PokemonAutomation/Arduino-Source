/*  Poke Portal Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PokePortalDetector_H
#define PokemonAutomation_PokemonSV_PokePortalDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonSV{



class PokePortalDetector : public StaticScreenDetector{
public:
    PokePortalDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Read where the cursor is. Returns -1 on failure.
    int detect_location(const ImageViewRGB32& screen) const;

    //  While sitting on the menu, move the cursor to the desired slot.
    //  Returns true if success.
    bool move_cursor(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        int row
    ) const;


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
