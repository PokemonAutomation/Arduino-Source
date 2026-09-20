/*  Hyperspace Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Helper functions for navigating in Hyperspace zones
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceNavigation_H
#define PokemonAutomation_PokemonLZA_HyperspaceNavigation_H

#include <cstdint>
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{

// Check if the current Calorie number on screen is <= min_calorie + additional_calorie_buffer
// Returns true if the condition is met, false otherwise
// Throws OperationFailedException if calorie cannot be detected
bool check_calorie(
    ConsoleHandle& console,
    ProControllerContext& context,
    uint16_t min_calorie,
    uint16_t additional_calorie_buffer = 0
);

// Wait until the warp pad is detected on screen
// Throws OperationFailedException if warp pad is not detected within 5 seconds
void detect_interactable(
    ConsoleHandle& console,
    ProControllerContext& context
);


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
