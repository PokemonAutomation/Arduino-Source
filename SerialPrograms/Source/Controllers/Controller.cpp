/*  Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Controller.h"

namespace PokemonAutomation{




struct AbstractController::Data{
    ListenerSet<InputSniffer> input_sniffers;
};
void AbstractController::add_input_sniffer(InputSniffer& listener){
    m_data->input_sniffers.add(listener);
}
void AbstractController::remove_input_sniffer(InputSniffer& listener){
    m_data->input_sniffers.remove(listener);
}
void AbstractController::on_command_input(WallClock timestamp, const ControllerState& state){
    m_data->input_sniffers.run_method(&InputSniffer::on_command_input, timestamp, state);
}


AbstractController::AbstractController()
    : m_data(CONSTRUCT_TOKEN)
{}
AbstractController::~AbstractController() = default;




void AbstractController::throw_bad_cast(const char* desired_typename){
    throw UserSetupError(
        logger(),
        std::string("Incompatible Controller:\n\n") +
        "Required:\n    " + desired_typename + "\n"
        "Actual:\n    " + name()
    );
}



}


