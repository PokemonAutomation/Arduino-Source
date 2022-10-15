/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "BoxDraw.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


BoxDraw_Descriptor::BoxDraw_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:BoxDraw",
        "Nintendo Switch", "Box Draw",
        "",
        "Test box coordinates for development.",
        FeedbackType::NONE, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

BoxDraw::BoxDraw()
    : X("<b>X Coordinate:</b>", 0.3, 0.0, 1.0)
    , Y("<b>Y Coordinate:</b>", 0.3, 0.0, 1.0)
    , WIDTH("<b>Width:</b>", 0.4, 0.0, 1.0)
    , HEIGHT("<b>Height:</b>", 0.4, 0.0, 1.0)
{
    PA_ADD_OPTION(X);
    PA_ADD_OPTION(Y);
    PA_ADD_OPTION(WIDTH);
    PA_ADD_OPTION(HEIGHT);
}

void BoxDraw::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    VideoOverlaySet set(env.console.overlay());

    set.add(COLOR_RED, {X, Y, WIDTH, HEIGHT});

    context.wait_until_cancel();
}




}
}
