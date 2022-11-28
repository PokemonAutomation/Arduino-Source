/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_MainMenuDetector_H
#define PokemonAutomation_PokemonSV_MainMenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
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
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  Read where the cursor is.
    //  The 2nd return value is the index. 0 is the top row. 6 is only valid on
    //  the left side and is Koraidon/Miraidon.
    std::pair<MenuSide, int> detect_location(const ImageViewRGB32& screen) const;

    //  While sitting on the menu, move the cursor to the desired slot.
    //  Returns true if success.
    bool move_cursor(
        ConsoleHandle& console, BotBaseContext& context,
        MenuSide side, int row
    ) const;


protected:
    Color m_color;
    GradientArrowDetector m_arrow_left;
    GradientArrowDetector m_arrow_right;
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
