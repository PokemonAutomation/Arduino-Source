/*  Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
// #include "CommonFramework/Exceptions/OperationFailedException.h"
#include "Common/Cpp/Exceptions.h"
#include "ControllerState.h"

namespace PokemonAutomation{




void ControllerState::load_json(const JsonObject& json){
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION,
        "This controller does not support serialization."
    );
}
JsonObject ControllerState::to_json() const{
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION,
        "This controller does not support serialization."
    );
}
void ControllerState::execute(
    Cancellable* scope,
    bool enable_logging,
    AbstractController& controller,
    Milliseconds duration
) const{
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION,
        "This controller does not support execution."
    );
}
std::string ControllerState::to_cpp(Milliseconds hold, Milliseconds release) const{
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION,
        "This controller does not support cpp conversion."
    );
}




}
