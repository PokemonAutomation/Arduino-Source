/*  Controller Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerConnection_H
#define PokemonAutomation_Controllers_ControllerConnection_H

#include <set>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ControllerDescriptor.h"

namespace PokemonAutomation{


class ControllerConnection{
public:
    struct StatusListener{
//        virtual void pre_connection_not_ready(ControllerConnection& connection){}
        virtual void post_connection_ready(
            ControllerConnection& connection,
            const std::map<ControllerType, std::set<ControllerFeature>>& controllers
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

    virtual std::map<ControllerType, std::set<ControllerFeature>> supported_controllers() const = 0;


protected:
    void set_status(const std::string& text);
    void declare_ready(const std::map<ControllerType, std::set<ControllerFeature>>& controllers);

private:
//    void signal_pre_not_ready();
    void signal_post_ready(const std::map<ControllerType, std::set<ControllerFeature>>& controllers);
    void signal_status_text_changed(const std::string& text);
    void signal_error(const std::string& text);


protected:
    std::atomic<bool> m_ready;

private:
    mutable SpinLock m_status_text_lock;
    std::string m_status_text;
    ListenerSet<StatusListener> m_status_listeners;
};




}
#endif
