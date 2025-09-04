/*  Controller Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerConnection_H
#define PokemonAutomation_Controllers_ControllerConnection_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ControllerDescriptor.h"

namespace PokemonAutomation{



struct ControllerModeStatus{
    ControllerType current_controller = ControllerType::None;
    std::vector<ControllerType> supported_controllers;
};



class ControllerConnection{
public:
    struct StatusListener{
//        virtual void pre_connection_not_ready(ControllerConnection& connection){}
        virtual void post_connection_ready(
            ControllerConnection& connection,
            const ControllerModeStatus& mode_status
        ){}
        virtual void status_text_changed(
            ControllerConnection& connection, const std::string& text
        ){}
        virtual void on_error(
            ControllerConnection& connection, const std::string& text
        ){};
    };

    void add_status_listener(StatusListener& listener);
    void remove_status_listener(StatusListener& listener);


public:
    virtual ~ControllerConnection() = default;

    bool is_ready() const{ return m_ready.load(std::memory_order_acquire); }
    std::string status_text() const;

    //  Returns the current controller type and the list of supported controllers.
    //  The current controller may be "None" if there is only one supported
    //  controller since it is implied to be that.
    virtual ControllerModeStatus controller_mode_status() const = 0;


public:
    void set_status_line0(const std::string& text, Color color = Color());
    void set_status_line1(const std::string& text, Color color = Color());


protected:
    void declare_ready(const ControllerModeStatus& mode_status);


private:
//    void signal_pre_not_ready();
    void signal_post_ready(const ControllerModeStatus& mode_status);
    void signal_status_text_changed(const std::string& text);
    void signal_error(const std::string& text);


protected:
    std::atomic<bool> m_ready;

private:
    mutable SpinLock m_status_text_lock;
    std::string m_status_line0;
    std::string m_status_line1;
    ListenerSet<StatusListener> m_status_listeners;
};




}
#endif
