/*  PABotBase2 Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/CancellableScope.h"
#include "PABotBase2_Connection.h"
#include "NintendoSwitch/Controllers/PABotBase2/NintendoSwitch_PABotBase2_OemController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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

void Connection::auto_select_controller_from_boot(){
//    cout << "auto_select_controller_from_boot()" << endl;
    ControllerType controller_type = ControllerType::NintendoSwitch_WiredProController;

    //  Already set to a controller.
    if (m_device->refresh_controller_type() != ControllerType::None){
        return;
    }

    //  Does not support the NS1 procon.
    auto iter = std::find(
        m_controller_list.begin(),
        m_controller_list.end(),
        controller_type
    );
    if (iter == m_controller_list.end()){
//        cout << "procon not supported: " << m_controller_list.size() << endl;
        return;
    }

    //  Firmware doesn't support querying session #.
    if (m_device->device_protocol_version() < 2026052101){
        return;
    }

    //  Not the first session.
    uint32_t session_num = m_device->query_u32(PABB2_MESSAGE_OPCODE_REQUEST_SESSION_NUM);
    m_device->logger().log("Session: " + tostr_u_commas(session_num));
    if (session_num != 0){
        return;
    }

    //  Now we can try to set the controller to NS1 procon.
    run_preconnect_configure(controller_type);
    m_device->try_set_controller_type(controller_type, false);

}





}
}
