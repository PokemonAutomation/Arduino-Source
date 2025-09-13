/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Controller.h"

namespace PokemonAutomation{



void AbstractController::throw_bad_cast(const char* desired_typename){
    throw UserSetupError(
        logger(),
        std::string("Incompatible Controller:\n\n") +
        "Required:\n    " + desired_typename + "\n"
        "Actual:\n    " + name()
    );
}



}


