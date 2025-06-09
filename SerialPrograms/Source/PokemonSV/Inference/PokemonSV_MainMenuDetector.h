/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MainMenuDetector_H
#define PokemonAutomation_PokemonSV_MainMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


enum class MenuSide{
    NONE,
    LEFT,
    RIGHT,
};


//  Detect the menu where your party is on the right while your main menu is on
//  the right.
class MainMenuDetector : public StaticScreenDetector{
public:
    MainMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  Read where the cursor is.
    //  The 2nd return value is the index. 0 is the top row. 6 is only valid on
    //  the left side and is Koraidon/Miraidon.
    std::pair<MenuSide, int> detect_location(const ImageViewRGB32& screen);

    //  While sitting on the menu, move the cursor to the desired slot.
    //  Returns true if success.
    //  If (fast = true) it will be faster, but may be unreliable. It may not
    //  actually land on the desired slot if the capture card is slow.
    bool move_cursor(
        const ProgramInfo& info,
        VideoStream& stream, ProControllerContext& context,
        MenuSide side, int row, bool fast = false
    );


protected:
    Color m_color;
    ImageFloatBox m_bottom;
    GradientArrowDetector m_arrow_left;
    GradientArrowDetector m_arrow_right;
    GradientArrowDetector m_dlc_icon;
};
class MainMenuWatcher : public DetectorToFinder<MainMenuDetector>{
public:
    MainMenuWatcher(Color color = COLOR_RED)
         : DetectorToFinder("MainMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};






}
}
}
#endif
