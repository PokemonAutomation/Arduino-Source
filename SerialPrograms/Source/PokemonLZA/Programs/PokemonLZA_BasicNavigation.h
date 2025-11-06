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


// Press button + to open the map.
// Will repeatedly pressing button + to ensure the map is opened
void open_map(ConsoleHandle& console, ProControllerContext& context);
// Starting at map view, press A to fast travel to the current selected fast travel location
// Return true if successful. Failure to fast travel can be:
// - Being attacked by wild pokemon
// - Current map cursor is not at a fast travel location.
// - In a main story session where fast travel is disabled.
bool fly_from_map(ConsoleHandle& console, ProControllerContext& context);

void sit_on_bench(ConsoleHandle& console, ProControllerContext& context);



}
}
}
#endif
