/*  Box Draw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_BoxDraw_H
#define PokemonAutomation_ConsoleInfra_BoxDraw_H

#include "CommonFramework/Options/NestedBoxDrawOption.h"
#include "ConsoleInfra/ConsolePanel.h"

namespace PokemonAutomation{
namespace ConsoleInfra{


class BoxDraw_Descriptor : public ConsolePanelDescriptor{
public:
    BoxDraw_Descriptor();
};

class BoxDraw : public ConsolePanelInstance{
public:
    BoxDraw(ConsoleSystemSession& system);

private:
    NestedBoxDrawOption BOX_DRAW;
};





}
}
#endif
