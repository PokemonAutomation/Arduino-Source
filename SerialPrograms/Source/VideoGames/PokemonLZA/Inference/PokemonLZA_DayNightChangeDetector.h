/*  Day Night Change Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DayNightChangeDetector_H
#define PokemonAutomation_PokemonLZA_DayNightChangeDetector_H

#include <optional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect day/night change
class DayNightChangeWatcher : public VisualInferenceCallback{
public:
    DayNightChangeWatcher(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    BlackScreenWatcher m_black_screen;
};




}
}
}
#endif
