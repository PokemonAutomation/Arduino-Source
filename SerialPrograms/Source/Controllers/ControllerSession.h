/*  Controller Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerSession_H
#define PokemonAutomation_Controllers_ControllerSession_H

#include <mutex>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ListenerSet.h"
//#include "Common/Cpp/Exceptions.h"
#include "Controller.h"
#include "ControllerDescriptor.h"
#include "ControllerConnection.h"

namespace PokemonAutomation{




class ControllerSession : private ControllerConnection::StatusListener{
public:
    struct Listener{
        virtual void ready_changed(bool ready){}
        virtual void descriptor_changed(
            const std::shared_ptr<const ControllerDescriptor>& descriptor
        ){}
        virtual void controller_changed(
            ControllerType controller_type,
            const std::vector<ControllerType>& available_controllers
        ){}
        virtual void post_status_text_changed(const std::string& text){}
        virtual void options_locked(bool locked){}
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    ~ControllerSession();
    ControllerSession(
        Logger& logger,
        ControllerOption& option
    );

    Logger& logger(){
        return m_logger;
    }
    std::vector<ControllerType> available_controllers() const;

    void get(ControllerOption& option);
    void set(const ControllerOption& option);

    bool ready() const;
    std::shared_ptr<const ControllerDescriptor> descriptor() const;
    ControllerType controller_type() const;
    std::string status_text() const;

    const ControllerOption& option() const{
        return m_option;
    }
    ControllerConnection& connection() const;
    AbstractController* controller() const;


public:
    //  Empty String: User input is allowed.
    //  Non-empty String: User input is blocked. The string is the reason.
    std::string user_input_blocked() const;
    void set_user_input_blocked(std::string disallow_reason);

    bool options_locked() const;
    void set_options_locked(bool locked);


public:
    bool set_device(const std::shared_ptr<const ControllerDescriptor>& device);
    bool set_controller(ControllerType controller_type);


public:
    //  Returns empty string on success. Otherwise returns error message.
    std::string reset(bool clear_settings);

    //  Try to run the following lambda on the underlying controller type.
    //  Returns empty string if successful. Otherwise returns error message.
    template <typename ControllerType, typename Lambda>
    std::string try_run(Lambda&& function) noexcept{
        ReadSpinLock lg(m_state_lock);
        if (!m_controller){
            return "Controller is null.";
        }
        try{
            //  This will be a cross-cast in most cases.
            ControllerType* child = dynamic_cast<ControllerType*>(m_controller.get());
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
    void make_controller(
        std::optional<ControllerType> change_controller,
        bool clear_settings
    );

//    virtual void pre_connection_not_ready(ControllerConnection& connection) override;
    virtual void post_connection_ready(ControllerConnection& connection) override;
    virtual void status_text_changed(
        ControllerConnection& connection,
        const std::string& text
    ) override;
    virtual void on_error(
        ControllerConnection& connection,
        const std::string& text
    ) override;

    void signal_ready_changed(bool ready);
    void signal_descriptor_changed(
        const std::shared_ptr<const ControllerDescriptor>& descriptor
    );
    void signal_controller_changed(
        ControllerType controller_type,
        const std::vector<ControllerType>& available_controllers
    );
    void signal_status_text_changed(const std::string& text);
    void signal_options_locked(bool locked);


private:
    Logger& m_logger;
    ControllerOption& m_option;

    std::mutex m_reset_lock;
    mutable SpinLock m_state_lock;

    bool m_options_locked;
    std::string m_user_input_disallow_reason;

    SpinLock m_message_lock;
    std::string m_controller_error;

    //  Next Reset
    ControllerType m_desired_controller;
    ControllerResetMode m_next_reset_mode;

    std::shared_ptr<const ControllerDescriptor> m_descriptor;
    std::unique_ptr<ControllerConnection> m_connection;
    std::unique_ptr<AbstractController> m_controller;

    ListenerSet<Listener> m_listeners;
};




}
#endif
