/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include <condition_variable>
#include "CommonFramework/Logging/Logger.h"
#include "Controllers/ControllerConnection.h"
#include "Controllers/SerialPABotBase/SerialPABotBase_Descriptor.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PybindSwitchController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class PybindSwitchProControllerInternal final : public ControllerConnection::StatusListener{
public:
    PybindSwitchProControllerInternal(const std::string& name)
        : m_logger(global_logger_raw(), "[Pybind]")
        , m_descriptor(name)
        , m_connection(m_descriptor.open_connection(m_logger, false))
    {
        m_connection->add_status_listener(*this);
    }
    ~PybindSwitchProControllerInternal(){
        m_connection->remove_status_listener(*this);
    }

    void wait_for_ready(){
        std::unique_lock<std::mutex> lg(m_wait_lock);
        m_cv.wait(lg, [this]{
            return m_procon.load(std::memory_order_acquire) && m_controller->is_ready();
        });
    }

    virtual void post_connection_ready(ControllerConnection& connection) override{
        m_controller = m_descriptor.make_controller(
            m_logger,
            connection,
            connection.current_controller(),
            ControllerResetMode::DO_NOT_RESET
        );
        ProController* procon = dynamic_cast<ProController*>(m_controller.get());
        if (procon == nullptr){
            m_error = "Incompatible controller type.";
            return;
        }
        m_procon.store(procon, std::memory_order_release);

        {
            std::unique_lock<std::mutex> lg(m_wait_lock);
        }
        m_cv.notify_all();
    }
    virtual void status_text_changed(
        ControllerConnection& connection, const std::string& text
    ) override{
        WriteSpinLock lg(m_lock);
        m_status = text;
    }
    virtual void on_error(
        ControllerConnection& connection, const std::string& text
    ) override{
        WriteSpinLock lg(m_lock);
        m_error = text;
    }

    ProController* controller(){
        return m_procon.load(std::memory_order_acquire);
    }


public:
    TaggedLogger m_logger;
    SerialPABotBase::SerialPABotBase_Descriptor m_descriptor;
    std::unique_ptr<ControllerConnection> m_connection;
    std::unique_ptr<AbstractController> m_controller;
    std::atomic<ProController*> m_procon;

    mutable SpinLock m_lock;
    std::string m_status;
    std::string m_error;

    std::mutex m_wait_lock;
    std::condition_variable m_cv;
};




PybindSwitchProController::PybindSwitchProController(const std::string& port_name){
    PybindSwitchProControllerInternal* internal = new PybindSwitchProControllerInternal(port_name);
    m_internals = internal;
    internal->wait_for_ready();
}
PybindSwitchProController::~PybindSwitchProController(){
    delete (PybindSwitchProControllerInternal*)m_internals;
}

bool PybindSwitchProController::is_ready() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        return false;
    }
    return controller->is_ready();
}
std::string PybindSwitchProController::current_status() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->m_connection->status_text();
}


void PybindSwitchProController::wait_for_all_requests(){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->wait_for_all(nullptr);
}
void PybindSwitchProController::wait(uint64_t duration){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_nop(nullptr, Milliseconds(duration));
}
void PybindSwitchProController::push_button(uint64_t delay, uint64_t hold, uint64_t release, uint32_t bitfield){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_buttons(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        (Button)bitfield
    );
}
void PybindSwitchProController::push_dpad(uint64_t delay, uint64_t hold, uint64_t release, uint8_t position){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_dpad(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        (DpadPosition)position
    );
}
void PybindSwitchProController::push_left_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_left_joystick(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        {x, y}
    );
}
void PybindSwitchProController::push_right_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_right_joystick(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        {x, y}
    );
}
void PybindSwitchProController::controller_state(
    uint64_t duration,
    uint32_t button_bitfield,
    uint8_t dpad_position,
    double left_x, double left_y,
    double right_x, double right_y
){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller();
    if (controller == nullptr){
        internal->m_logger.log("Controller is not ready.", COLOR_RED);
        return;
    }
    controller->issue_full_controller_state(
        nullptr,
        true,
        Milliseconds(duration),
        (Button)button_bitfield,
        (DpadPosition)dpad_position,
        {left_x, left_y},
        {right_x, right_y}
    );
}










}
}
