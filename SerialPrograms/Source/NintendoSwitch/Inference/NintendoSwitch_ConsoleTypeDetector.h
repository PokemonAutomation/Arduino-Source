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

namespace PokemonAutomation{
namespace NintendoSwitch{



enum class ConsoleTypeDetection{
    Unknown,
    Switch1,
    Switch2_Unknown,
    Switch2_International,
    Switch2_JapanLocked,
};
inline bool is_switch2(ConsoleTypeDetection detection){
    return detection == ConsoleTypeDetection::Switch2_Unknown
        || detection == ConsoleTypeDetection::Switch2_International
        || detection == ConsoleTypeDetection::Switch2_JapanLocked;
}




class ConsoleTypeDetector_Home{
public:
    ConsoleTypeDetector_Home(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleTypeDetection detect(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_bottom_line;
};

class ConsoleTypeDetector_StartGameUserSelect{
public:
    ConsoleTypeDetector_StartGameUserSelect(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    ConsoleTypeDetection detect(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_bottom_line;
};





}
}
#endif
