/*  Pybind Switch Controller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/Thread.h"
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



// Owns the serial connection and the controller object built on top of it.
//
// The connection runs its handshake on a background thread and calls
// `post_connection_ready()` once the device reports its controller type. The device
// may also reset and handshake again (e.g. after a USB hiccup), in which case the
// controller object is rebuilt. `m_procon` always points at the current controller,
// or is null while there is none.
class PybindSwitchProControllerInternal final : public ControllerConnection::StatusListener{
public:
    PybindSwitchProControllerInternal(const std::string& name)
        : m_logger(global_logger_raw(), "Pybind")
        , m_descriptor(name)
        , m_connection(m_descriptor.open_connection(m_logger))
    {
        m_connection->add_status_listener(*this);
    }
    ~PybindSwitchProControllerInternal(){
        m_connection->remove_status_listener(*this);
        //  Destroy the controller before the connection it references.
        m_procon.store(nullptr, std::memory_order_release);
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_controller.reset();
        }
        m_connection.reset();
    }

    bool wait_for_ready(uint64_t timeout_millis){
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait_for(lg, Milliseconds(timeout_millis), [this]{
            ProController* procon = m_procon.load(std::memory_order_acquire);
            return m_failed || (procon != nullptr && procon->is_ready());
        });
        return controller_if_ready() != nullptr;
    }

    virtual void post_connection_ready(ControllerConnection& connection) override{
//        cout << "post_connection_ready()" << endl;

        std::unique_ptr<AbstractController> controller = m_descriptor.make_controller(
            m_logger,
            connection,
            connection.current_controller()
        );
        ProController* procon = dynamic_cast<ProController*>(controller.get());
        if (procon == nullptr){
//            cout << "post_connection_ready() - incompatible" << endl;
            m_connection->set_status_line1(
                "Incompatible controller type. Use the main program to set the device "
                "to a Pro Controller or a wired controller.",
                COLOR_RED
            );
        }

        {
            std::lock_guard<Mutex> lg(m_lock);
            m_procon.store(nullptr, std::memory_order_release);
            m_controller = std::move(controller);
            m_procon.store(procon, std::memory_order_release);
            m_failed = procon == nullptr;
        }
        m_cv.notify_all();
    }
    virtual void on_error(ControllerConnection& connection, const std::string& text) override{
        {
            std::lock_guard<Mutex> lg(m_lock);
            m_failed = true;
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
            throw InvalidConnectionStateException(
                "Controller is not ready: " + m_connection->status_text()
            );
        }
        return *procon;
    }


public:
    TaggedLogger m_logger;
    SerialPABotBase::SerialPABotBase2_Descriptor m_descriptor;
    std::unique_ptr<ControllerConnection> m_connection;

    Mutex m_lock;
    ConditionVariable m_cv;
    std::unique_ptr<AbstractController> m_controller;
    std::atomic<ProController*> m_procon{nullptr};
    bool m_failed = false;
};




PybindSwitchProController::PybindSwitchProController(const std::string& port_name){
    PybindSwitchProControllerInternal* internal = new PybindSwitchProControllerInternal(port_name);
    m_internals = internal;
}
PybindSwitchProController::~PybindSwitchProController(){
    delete (PybindSwitchProControllerInternal*)m_internals;
}

bool PybindSwitchProController::wait_for_ready(uint64_t timeout_millis){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->wait_for_ready(timeout_millis);
}

bool PybindSwitchProController::is_ready() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    return internal->controller_if_ready() != nullptr;
}
std::string PybindSwitchProController::current_status() const{
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    //  The status is formatted for the GUI with HTML tags such as
    //  "<font color=...>...</font>". Strip them for plain-text callers.
    std::string html = internal->m_connection->status_text();
    std::string text;
    bool in_tag = false;
    for (char ch : html){
        if (ch == '<'){
            in_tag = true;
        }else if (ch == '>'){
            in_tag = false;
        }else if (!in_tag){
            text += ch;
        }
    }
    return text;
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
void PybindSwitchProController::cancel_all(){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller_if_ready();
    if (controller == nullptr){
        return;
    }
    controller->cancel_all_commands();
}
bool PybindSwitchProController::release_all(uint64_t timeout_millis){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    ProController* controller = internal->controller_if_ready();
    if (controller == nullptr){
        return false;
    }
    controller->cancel_all_commands();

    //  Bound the confirmation: this timer cancels `scope` when the timeout passes,
    //  which makes `issue_nop()` / `wait_for_all()` below throw
    //  OperationCancelledException.
    CancellableHolder<CancellableScope> scope;
    Mutex lock;
    ConditionVariable cv;
    bool done = false;
    Thread timer([&]{
        std::unique_lock<Mutex> lg(lock);
        if (!cv.wait_for(lg, Milliseconds(timeout_millis), [&]{ return done; })){
            scope.cancel(nullptr);
        }
    });

    bool confirmed = false;
    try{
        //  Queued after the cancel, so the device reports this no-op finished only
        //  after it has dropped everything before it and held neutral for 10 ms.
        controller->issue_nop(&scope, Milliseconds(10));
        controller->wait_for_all(&scope);
        confirmed = true;
    }catch (OperationCancelledException&){}

    {
        std::lock_guard<Mutex> lg(lock);
        done = true;
    }
    cv.notify_all();
    timer.join();

    if (!confirmed){
        internal->m_logger.log(
            "release_all(): device did not confirm the neutral state within " +
            std::to_string(timeout_millis) + " ms.",
            COLOR_RED
        );
    }
    return confirmed;
}
void PybindSwitchProController::wait(uint64_t duration){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_nop(nullptr, Milliseconds(duration));
}
void PybindSwitchProController::push_button(uint64_t delay, uint64_t hold, uint64_t release, uint32_t bitfield){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_buttons(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        (Button)bitfield
    );
}
void PybindSwitchProController::push_dpad(uint64_t delay, uint64_t hold, uint64_t release, uint8_t position){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_dpad(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        (DpadPosition)position
    );
}
void PybindSwitchProController::push_left_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_left_joystick(
        nullptr,
        Milliseconds(delay),
        Milliseconds(hold),
        Milliseconds(release),
        {x, y}
    );
}
void PybindSwitchProController::push_right_joystick(uint64_t delay, uint64_t hold, uint64_t release, double x, double y){
    PybindSwitchProControllerInternal* internal = (PybindSwitchProControllerInternal*)m_internals;
    internal->controller().issue_right_joystick(
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
    internal->controller().issue_full_controller_state(
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
