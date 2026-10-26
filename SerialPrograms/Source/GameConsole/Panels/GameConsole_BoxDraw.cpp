/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GameConsole/Framework/ConsoleSystemSession.h"
#include "GameConsole_BoxDraw.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace GameConsole{



BoxDraw_Descriptor::BoxDraw_Descriptor()
    : ConsolePanelDescriptor(
        "GameConsole:BoxDraw",
        "Game Console", "Box Draw",
        "",
        "Test box coordinates for development.",
        PanelQualityStanding::DEVELOPER,
        false
    )
{}


BoxDraw::BoxDraw(ConsoleSystemSession& system)
    : BOX_DRAW(LockMode::UNLOCK_WHILE_RUNNING, system.overlay())
{
    PA_ADD_OPTION(BOX_DRAW);
}








}
}
