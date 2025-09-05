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



class ControllerConnection{
public:
    struct StatusListener{
//        virtual void pre_connection_not_ready(ControllerConnection& connection){}
        virtual void post_connection_ready(ControllerConnection& connection){}
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
    ControllerConnection()
        : m_current_controller(ControllerType::None)
        , m_ready(false)
    {}
    virtual ~ControllerConnection() = default;

    ControllerType current_controller() const{
        return m_current_controller.load(std::memory_order_acquire);
    }
    bool is_ready() const{ return m_ready.load(std::memory_order_acquire); }
    std::string status_text() const;

    //  It it not safe to call this until "is_ready()" is true.
    const std::vector<ControllerType>& controller_list(){
        return m_controller_list;
    }



public:
    void set_status_line0(const std::string& text, Color color = Color());
    void set_status_line1(const std::string& text, Color color = Color());


protected:
    void declare_ready();


private:
//    void signal_pre_not_ready();
    void signal_post_ready();
    void signal_status_text_changed(const std::string& text);
    void signal_error(const std::string& text);


protected:
    //  This is written to once and never modified again.
    std::vector<ControllerType> m_controller_list;

    std::atomic<ControllerType> m_current_controller;
    std::atomic<bool> m_ready;


private:
    mutable SpinLock m_status_text_lock;
    std::string m_status_line0;
    std::string m_status_line1;
    ListenerSet<StatusListener> m_status_listeners;
};




}
#endif
