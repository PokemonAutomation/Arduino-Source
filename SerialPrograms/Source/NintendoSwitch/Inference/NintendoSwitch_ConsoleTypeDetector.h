/*  Console Type Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleTypeDetector_H
#define PokemonAutomation_NintendoSwitch_ConsoleTypeDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class ConsoleTypeDetector_Home{
public:
    ConsoleTypeDetector_Home(ConsoleState& state, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleType detect(const ImageViewRGB32& screen);

private:
    ConsoleState& m_state;
    Color m_color;
    ImageFloatBox m_bottom_line;
};

class ConsoleTypeDetector_StartGameUserSelect{
public:
    ConsoleTypeDetector_StartGameUserSelect(ConsoleState& state, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleType detect(const ImageViewRGB32& screen);

private:
    ConsoleState& m_state;
    Color m_color;
    ImageFloatBox m_bottom_line;
};





}
}
#endif
