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
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class ConsoleTypeDetector_Home{
public:
    ConsoleTypeDetector_Home(ConsoleHandle& console, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleType detect_only(const ImageViewRGB32& screen);
    void commit_to_cache();

private:
    ConsoleHandle& m_console;
    Color m_color;
    ImageFloatBox m_bottom_line;
    ConsoleType m_last;
};

class ConsoleTypeDetector_StartGameUserSelect{
public:
    ConsoleTypeDetector_StartGameUserSelect(ConsoleHandle& console, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleType detect_only(const ImageViewRGB32& screen);
    void commit_to_cache();

private:
    ConsoleHandle& m_console;
    Color m_color;
    ImageFloatBox m_bottom_line;
    ConsoleType m_last;
};

ConsoleType detect_console_type_from_in_game(ConsoleHandle& console, ProControllerContext& context);



}
}
#endif
