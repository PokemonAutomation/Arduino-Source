/*  Controller Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerSession_H
#define PokemonAutomation_Controllers_ControllerSession_H

#include <mutex>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ListenerSet.h"
//#include "Common/Cpp/Exceptions.h"
#include "ControllerDescriptor.h"
#include "ControllerConnection.h"

namespace PokemonAutomation{



class ControllerSession{
public:
    struct Listener : ControllerConnection::StatusListener{
        virtual void ready_changed(bool ready){}
        virtual void controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor){}
        virtual void status_text_changed(const std::string& text){}
        virtual void options_locked(bool locked){}
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    ~ControllerSession();
    ControllerSession(
        Logger& logger,
        ControllerOption& option,
        const ControllerRequirements& requirements
    );

    Logger& logger(){
        return m_logger;
    }
    const ControllerRequirements& requirements(){
        return m_requirements;
    }

    void get(ControllerOption& option);
    void set(const ControllerOption& option);

    bool ready() const;
    std::shared_ptr<const ControllerDescriptor> descriptor() const;
    std::string status_text() const;

    const ControllerOption& option() const{
        return m_option;
    }
    ControllerConnection& connection() const;


public:
    //  Empty String: User input is allowed.
    //  Non-empty String: User input is blocked. The string is the reason.
    std::string user_input_blocked() const;
    void set_user_input_blocked(std::string disallow_reason);

    bool options_locked() const;
    void set_options_locked(bool locked);

public:
    bool set_device(const std::shared_ptr<const ControllerDescriptor>& device);


public:
    //  Returns empty string on success. Otherwise returns error message.
    std::string reset();

    //  Try to run the following lambda on the underlying controller type.
    //  Returns empty string if successful. Otherwise returns error message.
    template <typename ControllerType, typename Lambda>
    std::string try_run(Lambda&& function) noexcept{
        std::lock_guard<std::mutex> lg(m_state_lock);
        if (!m_connection){
            return "Connection is null.";
        }
        try{
            //  This will be a cross-cast in most cases.
            ControllerType* child = dynamic_cast<ControllerType*>(m_connection.get());
            if (child == nullptr){
                m_logger.log("ControllerSession::try_run(): Incompatible controller type cast.", COLOR_RED);
                return "Incompatible controller type cast.";
            }
            if (!m_user_input_disallow_reason.empty()){
                return m_user_input_disallow_reason;
            }
            function(*child);
        }catch (Exception& e){
            return e.to_str();
        }catch (...){
            return "Unknown exception.";
        }
        return "";
    }


private:
    void signal_ready_changed(bool ready);
    void signal_controller_changed(const std::shared_ptr<const ControllerDescriptor>& descriptor);
    void signal_status_text_changed(const std::string& text);
    void signal_options_locked(bool locked);


private:
    Logger& m_logger;
    const ControllerRequirements& m_requirements;
    ControllerOption& m_option;

    mutable std::mutex m_state_lock;
    bool m_options_locked;
    std::string m_user_input_disallow_reason;
    std::shared_ptr<const ControllerDescriptor> m_descriptor;
    std::unique_ptr<ControllerConnection> m_connection;

    ListenerSet<Listener> m_listeners;
};




}
#endif
