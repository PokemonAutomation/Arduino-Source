/*  Hardware Validation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_HardwareValidation_H
#define PokemonAutomation_HardwareValidation_H

namespace PokemonAutomation{

// Check user hardware.
// If the hardware is too old, send a QMessageBox to tell user and return false.
// If the hardware may not be powerful enough, send a QMessageBox to inform user.
bool check_hardware();


}
#endif
