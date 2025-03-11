/*  Panic Dumping
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_PanicDumping_H
#define PokemonAutomation_PanicDumping_H

#include <functional>

namespace PokemonAutomation{


void panic_dump(const char* location, const char* message);

void run_with_catch(const char* location, std::function<void()>&& lambda);


}
#endif

