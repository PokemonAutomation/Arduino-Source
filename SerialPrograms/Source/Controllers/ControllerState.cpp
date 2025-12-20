/*  Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "ControllerState.h"

namespace PokemonAutomation{




void ControllerState::load_json(const JsonObject& json){
    throw OperationFailedException(
        ErrorReport::NO_ERROR_REPORT,
        "This controller does not support serialization."
    );
}
JsonObject ControllerState::to_json() const{
    throw OperationFailedException(
        ErrorReport::NO_ERROR_REPORT,
        "This controller does not support serialization."
    );
}
void ControllerState::execute(
    Cancellable* scope,
    bool enable_logging,
    AbstractController& controller,
    Milliseconds duration
) const{
    throw OperationFailedException(
        ErrorReport::NO_ERROR_REPORT,
        "This controller does not support execution."
    );
}
std::string ControllerState::to_cpp(Milliseconds hold, Milliseconds release) const{
    throw OperationFailedException(
        ErrorReport::NO_ERROR_REPORT,
        "This controller does not support cpp conversion."
    );
}




}
