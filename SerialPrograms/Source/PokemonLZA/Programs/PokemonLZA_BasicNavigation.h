/*  Basic Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BasicNavigation_H
#define PokemonAutomation_PokemonLZA_BasicNavigation_H

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{


void ensure_map(ConsoleHandle& console, ProControllerContext& context);
void open_map(ConsoleHandle& console, ProControllerContext& context);
bool fly_from_map(ConsoleHandle& console, ProControllerContext& context);

void sit_on_bench(ConsoleHandle& console, ProControllerContext& context);



}
}
}
#endif
