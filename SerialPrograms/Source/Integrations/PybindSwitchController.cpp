/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/Logging/TaggedLogger.h"
#include "Controllers/ControllerConnection.h"
#include "Controllers/PABotBase2/SerialPABotBase2_Descriptor.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PybindSwitchController.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



class PybindSwitchProControllerInternal final : public ControllerConnection::StatusListener{
public:
    PybindSwitchProControllerInternal(const std::string& name)
        : m_logger(global_logger_raw(), "Pybind")
        , m_descriptor(name)
        , m_connection(m_descriptor.open_connection(m_logger))
    {
        //  open_connection() returns nullptr for an empty port name.
        if (m_connection){
            m_connection->add_status_listener(*this);
        }
    }
    ~PybindSwitchProControllerInternal(){
        m_connection->remove_status_listener(*this);
    }

    bool wait_for_ready(uint64_t timeout_ms){
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait_for(lg, Milliseconds(timeout_ms), [this]{
            return m_connected;
        });
        return controller_if_ready() != nullptr;
    }

    virtual void post_connection_ready(ControllerConnection& connection) override{
//        cout << "post_connection_ready()" << endl;

        m_controller = m_descriptor.make_controller(
            m_logger,
            connection,
            connection.current_controller()
        );
        ProController* procon = dynamic_cast<ProController*>(m_controller.get());
        if (procon == nullptr){
//            cout << "post_connection_ready() - incompatible" << endl;
            m_connection->set_status_line1(
                "Incompatible controller type. Use the main program to set the device "
                "to a Pro Controller or a wired controller.",
                COLOR_RED
            );
        }else{
//            cout << "post_connection_ready() - good" << endl;
            m_procon.store(procon, std::memory_order_release);
        }

        {
            std::unique_lock<Mutex> lg(m_lock);
            m_connected = true;
        }
        m_cv.notify_all();
    }

    ProController* controller_if_ready(){
        ProController* procon = m_procon.load(std::memory_order_acquire);
        if (procon == nullptr || !procon->is_ready()){
            return nullptr;
        }
        return procon;
    }

    // Return the current controller, or throw InvalidConnectionStateException if
    // there is none.
    ProController& controller(){
        ProController* procon = controller_if_ready();
        if (procon == nullptr){
            std::string status = m_connection ? m_connection->status_text() : "No connection.";
            throw InvalidConnectionStateException("Controller is not ready: " + status);
        }
        return *procon;
    }


public:
    TaggedLogger m_logger;
    SerialPABotBase::SerialPABotBase2_Descriptor m_descriptor;
    std::unique_ptr<ControllerConnection> m_connection;
    std::unique_ptr<AbstractController> m_controller;
    std::atomic<ProController*> m_procon;

    bool m_connected = false;
    Mutex m_lock;
    ConditionVariable m_cv;
};




PybindSwitchProController::PybindSwitchProController(const std::string& port_name){
    PybindSwitchProControllerInternal* internal = new PybindSwitchProControllerInternal(port_name);
    m_internals = internal;
}
PybindSwitchProController::~PybindSwitchProController(){
    delete (PybindSwitchProControllerInternal*)m_internals;
}

bool PybindSwitchProController::wait_for_ready(uint64_t timeout_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->wait_for_ready(timeout_ms);
}

bool PybindSwitchProController::is_ready() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->controller_if_ready() != nullptr;
}
std::string PybindSwitchProController::current_status() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->m_connection ? internal->m_connection->raw_status_text() : "No connection.";
}
std::string PybindSwitchProController::controller_name() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller_if_ready();
    return controller == nullptr ? "" : controller->name();
}


void PybindSwitchProController::wait_for_all_requests(){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller_if_ready();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->wait_for_all(nullptr);
}
bool PybindSwitchProController::cancel_all_and_wait(uint64_t timeout_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller_if_ready();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return false;
    }
    return controller->cancel_all_and_wait(Milliseconds(timeout_ms));
}
void PybindSwitchProController::wait(uint64_t duration_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_nop(nullptr, Milliseconds(duration_ms));
}
void PybindSwitchProController::push_button(uint32_t button_bitfield, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_buttons(
        nullptr,
        Milliseconds(delay_ms),
        Milliseconds(hold_ms),
        Milliseconds(release_ms),
        (Button)button_bitfield
    );
}
void PybindSwitchProController::push_dpad(uint8_t dpad_position, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_dpad(
        nullptr,
        Milliseconds(delay_ms),
        Milliseconds(hold_ms),
        Milliseconds(release_ms),
        (DpadPosition)dpad_position
    );
}
void PybindSwitchProController::push_left_joystick(double x, double y, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_left_joystick(
        nullptr,
        Milliseconds(delay_ms),
        Milliseconds(hold_ms),
        Milliseconds(release_ms),
        JoystickPosition(x, y)
    );
}
void PybindSwitchProController::push_right_joystick(double x, double y, uint64_t delay_ms, uint64_t hold_ms, uint64_t release_ms){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_right_joystick(
        nullptr,
        Milliseconds(delay_ms),
        Milliseconds(hold_ms),
        Milliseconds(release_ms),
        JoystickPosition(x, y)
    );
}
void PybindSwitchProController::controller_state(
    uint32_t button_bitfield,
    uint8_t dpad_position,
    double left_x, double left_y,
    double right_x, double right_y,
    uint64_t duration_ms
){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    //  Same as pbf_controller_state().
    internal->controller().issue_full_controller_state(
        nullptr,
        true,
        Milliseconds(duration_ms),
        (Button)button_bitfield,
        (DpadPosition)dpad_position,
        {left_x, left_y},
        {right_x, right_y}
    );
}










}
}
