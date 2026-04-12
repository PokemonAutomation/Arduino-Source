/*  PABotBase2 Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/CancellableScope.h"
#include "PABotBase2_Connection.h"
#include "NintendoSwitch/Controllers/PABotBase2/NintendoSwitch_PABotBase2_OemController.h"


namespace PokemonAutomation{
namespace PABotBase2{



void Connection::run_preconnect_configure(ControllerType controller_type){
    //  If the controller has settings, set them now.
    switch (controller_type){
    case ControllerType::NintendoSwitch_WiredProController:
    case ControllerType::NintendoSwitch_WiredLeftJoycon:
    case ControllerType::NintendoSwitch_WiredRightJoycon:
    case ControllerType::NintendoSwitch_WirelessProController:
    case ControllerType::NintendoSwitch_WirelessLeftJoycon:
    case ControllerType::NintendoSwitch_WirelessRightJoycon:
        NintendoSwitch::PABotBase2_OemController::run_preconnect_configure(
            m_device->logger(), *this, controller_type
        );
        break;
    default:;
    }

}
void Connection::try_set_controller_type(
    ControllerType controller_type,
    bool clear_settings
) noexcept{
    if (!is_ready()){
        return;
    }

    try{
        run_preconnect_configure(controller_type);
    }catch (...){}

    m_device->try_set_controller_type(controller_type, clear_settings);
}


}
}
