/*  Day Night Change Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA_DayNightChangeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


DayNightChangeWatcher::DayNightChangeWatcher(Color color)
: VisualInferenceCallback("DayNighChangeWatcher")
, m_black_screen(color) {}

void DayNightChangeWatcher::make_overlays(VideoOverlaySet& items) const{
    m_black_screen.make_overlays(items);
}

bool DayNightChangeWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_black_screen.process_frame(frame, timestamp);
}


}
}
}
