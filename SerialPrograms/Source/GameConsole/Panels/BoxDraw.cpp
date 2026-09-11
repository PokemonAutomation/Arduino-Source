/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GameConsole/ConsoleSystemSession.h"
#include "BoxDraw.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace GameConsole{



BoxDraw_Descriptor::BoxDraw_Descriptor()
    : ConsolePanelDescriptor(
        Color(),
        "GameConsole:BoxDraw",
        "Game Console", "Box Draw",
        "",
        "Test box coordinates for development."
    )
{}


BoxDraw::BoxDraw(ConsoleSystemSession& system)
    : BOX_DRAW(LockMode::UNLOCK_WHILE_RUNNING, system.overlay())
{
    PA_ADD_OPTION(BOX_DRAW);
}








}
}
