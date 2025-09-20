/*  Keyboard Input
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_KeyboardInput_H
#define PokemonAutomation_Controllers_KeyboardInput_H

#include <map>
#include <thread>
#include <condition_variable>
#include <Qt>
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Controllers/Controller.h"
#include "Controllers/KeyboardInput/GlobalQtKeyMap.h"
#include "KeyboardEventHandler.h"
#include "KeyboardStateTracker.h"

class QKeyEvent;

namespace PokemonAutomation{


class ControllerSession;




class ControllerState{
public:
    virtual ~ControllerState() = default;

    virtual void clear() = 0;

    virtual bool operator==(const ControllerState& x) const = 0;
    virtual bool operator!=(const ControllerState& x) const{ return !(*this == x); }

    virtual bool is_neutral() const = 0;

    virtual JsonObject serialize_state() const {return JsonObject();};
};




class KeyboardInputController{
public:
    KeyboardInputController(Logger& logger, bool enabled);
    virtual ~KeyboardInputController();


public:
    void clear_state();

    void on_key_press(const QKeyEvent& key);
    void on_key_release(const QKeyEvent& key);


protected:
    void start();           //  Child class must call this at end of constructor.
    void stop() noexcept;   //  Child class must call this at start of destructor.

    virtual std::unique_ptr<ControllerState> make_state() const = 0;
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) = 0;
    virtual void cancel_all_commands() = 0;
    virtual void replace_on_next_command() = 0;
    virtual void send_state(const ControllerState& state) = 0;

private:
    void thread_loop();


private:
    Logger& m_logger;

    //  Controller State
    SpinLock m_state_lock;
    KeyboardStateTracker m_state_tracker;

    std::atomic<bool> m_stop;

    std::mutex m_sleep_lock;
    std::condition_variable m_cv;
    std::thread m_thread;
};




template <typename StateType, typename DeltaType>
class KeyboardManager : public KeyboardInputController, public KeyboardEventHandler{
public:
    KeyboardManager(Logger& logger, AbstractController& controller)
        : KeyboardInputController(logger, true)
        , m_controller(&controller)
    {}
    void stop() noexcept{
        {
            WriteSpinLock lg(m_lock);
            if (m_controller == nullptr){
                return;
            }
            m_controller = nullptr;
        }
        KeyboardInputController::stop();
    }

    virtual std::unique_ptr<ControllerState> make_state() const override{
        return std::make_unique<StateType>();
    }
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) override{
        DeltaType deltas;
        const QtKeyMap& qkey_map = QtKeyMap::instance();
        for (uint32_t native_key : pressed_keys){
            std::set<QtKeyMap::QtKey> qkeys = qkey_map.get_QtKeys(native_key);
            for (QtKeyMap::QtKey qkey : qkeys){
                auto iter = m_mapping.find(qkey.key);
                if (iter != m_mapping.end()){
                    deltas += iter->second;
                    break;
                }
            }
        }
        deltas.to_state(static_cast<StateType&>(state));
    }
    virtual void cancel_all_commands() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        WallClock time_stamp = current_time();
        m_controller->cancel_all_commands();
        report_keyboard_command_stopped(time_stamp);
    }
    virtual void replace_on_next_command() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->replace_on_next_command();
    }


protected:
    SpinLock m_lock;
    AbstractController* m_controller;
    std::map<Qt::Key, DeltaType> m_mapping;
};




}
#endif
