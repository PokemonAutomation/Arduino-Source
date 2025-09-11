/*  HID Keyboard
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/SerialPABotBase/SerialPABotBase_Messages_HID_Keyboard.h"
#include "CommonTools/Async/InterruptableCommands.tpp"
#include "CommonTools/Async/SuperControlSession.tpp"
#include "Controllers/KeyboardInput/KeyboardInput.h"
#include "HID_Keyboard.h"

namespace PokemonAutomation{

//  Instantiate some template helper classes.
template class AsyncCommandSession<HidControllers::Keyboard>;
template class SuperControlSession<HidControllers::Keyboard>;

namespace HidControllers{

using namespace std::chrono_literals;


const char Keyboard::NAME[] = "HID: Keyboard";


#if 0

const std::map<Qt::Key, uint8_t>& KEYID_TO_HID_KEY(){
    static const std::map<Qt::Key, uint8_t> database{
        {Qt::Key::Key_A, 0x04},
        {Qt::Key::Key_B, 0x05},
        {Qt::Key::Key_C, 0x06},
        {Qt::Key::Key_D, 0x07},
        {Qt::Key::Key_E, 0x08},
        {Qt::Key::Key_F, 0x09},
        {Qt::Key::Key_G, 0x0a},
        {Qt::Key::Key_H, 0x0b},
        {Qt::Key::Key_I, 0x0c},
        {Qt::Key::Key_J, 0x0d},
        {Qt::Key::Key_K, 0x0e},
        {Qt::Key::Key_L, 0x0f},
        {Qt::Key::Key_M, 0x10},
        {Qt::Key::Key_N, 0x11},
        {Qt::Key::Key_O, 0x12},
        {Qt::Key::Key_P, 0x13},
        {Qt::Key::Key_Q, 0x14},
        {Qt::Key::Key_R, 0x15},
        {Qt::Key::Key_S, 0x16},
        {Qt::Key::Key_T, 0x17},
        {Qt::Key::Key_U, 0x18},
        {Qt::Key::Key_V, 0x19},
        {Qt::Key::Key_W, 0x1a},
        {Qt::Key::Key_X, 0x1b},
        {Qt::Key::Key_Y, 0x1c},
        {Qt::Key::Key_Z, 0x1d},

        {Qt::Key::Key_1,        0x1e},
        {Qt::Key::Key_2,        0x1f},
        {Qt::Key::Key_3,        0x20},
        {Qt::Key::Key_4,        0x21},
        {Qt::Key::Key_5,        0x22},
        {Qt::Key::Key_6,        0x23},
        {Qt::Key::Key_7,        0x24},
        {Qt::Key::Key_8,        0x25},
        {Qt::Key::Key_9,        0x26},
        {Qt::Key::Key_0,        0x27},

        {Qt::Key::Key_Exclam,   0x1e},
        {Qt::Key::Key_At,       0x1f},
    };
    return database;
};




class KeyboardState final : public ControllerState{
public:
    virtual void clear() override{
        m_pressed.clear();
        m_order.clear();
    }
    virtual bool operator==(const ControllerState& x) const override{
        if (typeid(*this) != typeid(x)){
            return false;
        }
        const KeyboardState& r = static_cast<const KeyboardState&>(x);
        if (m_pressed.size() != r.m_pressed.size()){
            return false;
        }
        auto iter0 = m_pressed.begin();
        auto iter1 = r.m_pressed.begin();
        for (; iter0 != m_pressed.end(); ++iter0, ++iter1){
            if (iter0->first != iter1->first){
                return false;
            }
        }
        return true;
    }
    virtual bool is_neutral() const override{
        return m_pressed.empty();
    }

    auto begin(){
        return m_pressed.begin();
    }
    auto end(){
        return m_pressed.end();
    }
    void erase(Qt::Key key){
        auto iter = m_pressed.find(key);
        m_order.erase(iter->second);
        m_pressed.erase(iter);
    }
    void add_key(WallClock timestamp, Qt::Key key){
        auto ret = m_pressed.try_emplace(key);
        if (!ret.second){
            return;
        }
        try{
            auto iter = m_order.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(timestamp),
                std::forward_as_tuple(key)
            );
            ret.first->second = iter;
        }catch (...){
            m_pressed.erase(ret.first);
            throw;
        }
    }

    uint64_t to_report() const{
        union{
            pabb_HID_Keyboard_State report;
            uint64_t u64;
        };
        u64 = 0;

        report.modifiers = 0;
        if (m_pressed.contains(Qt::Key::Key_Control)){
            report.modifiers |= 1;
        }
        if (m_pressed.contains(Qt::Key::Key_Shift)){
            report.modifiers |= 2;
        }
        if (m_pressed.contains(Qt::Key::Key_Alt)){
            report.modifiers |= 4;
        }
        if (m_pressed.contains(Qt::Key::Key_Meta)){
            report.modifiers |= 8;
        }

        const std::map<Qt::Key, uint8_t>& hid_map = KEYID_TO_HID_KEY();

        auto iter = m_order.begin();
        size_t c = 0;
        for (; c < 6 && iter != m_order.end(); c++, ++iter){
            auto iter0 = hid_map.find(iter->second);
            if (iter0 != hid_map.end()){
                report.key[c++] = iter0->second;
            }
        }

        return u64;
    }

private:
    std::map<Qt::Key, std::multimap<WallClock, Qt::Key>::iterator> m_pressed;
    std::multimap<WallClock, Qt::Key> m_order;
};


class Keyboard::KeyboardManager final : public PokemonAutomation::KeyboardInputController{
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
        return std::make_unique<KeyboardState>();
    }
    virtual void update_state(ControllerState& state, const std::set<uint32_t>& pressed_keys) override{
        const QtKeyMap& qkey_map = QtKeyMap::instance();
//        const std::map<Qt::Key, uint8_t>& hid_map = KEYID_TO_HID_KEY();

        KeyboardState& lstate = static_cast<KeyboardState&>(state);

        std::set<Qt::Key> pressed;
        for (uint32_t native_key : pressed_keys){
            std::set<Qt::Key> qkeys = qkey_map.get_QtKeys(native_key);
            for (Qt::Key qkey : qkeys){
                pressed.insert(qkey);
#if 0
                auto iter = hid_map.find(qkey);
                if (iter != hid_map.end()){
                    pressed.insert(iter->second);
                }
#endif
            }
        }

        //  Remove keys that are no longer in the pressed set.
        for (auto& item : lstate){
            if (pressed.find(item.first) == pressed.end()){
                lstate.erase(item.first);
            }
        }

        WallClock now = current_time();
        for (Qt::Key key : pressed){
            lstate.add_key(now, key);
        }
    }
    virtual void cancel_all_commands() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->cancel_all_commands();
    }
    virtual void replace_on_next_command() override{
        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }
        m_controller->replace_on_next_command();
    }

    virtual void send_state(const ControllerState& state) override{
        const KeyboardState& lstate = static_cast<const KeyboardState&>(state);

        WriteSpinLock lg(m_lock);
        if (m_controller == nullptr){
            return;
        }

        uint64_t report = lstate.to_report();
        static_cast<Keyboard*>(m_controller)->issue_report(
            nullptr, 2000ms, report
        );
    }


protected:
    SpinLock m_lock;
    AbstractController* m_controller;
};

#endif



Keyboard::Keyboard(Logger& logger)
//    : m_keyboard_manager(CONSTRUCT_TOKEN, logger, *this)
{

}
Keyboard::~Keyboard(){

}
void Keyboard::stop() noexcept{
//    m_keyboard_manager->stop();
}


void Keyboard::keyboard_release_all(){
//    m_keyboard_manager->clear_state();
}
void Keyboard::keyboard_press(const QKeyEvent& event){
//    m_keyboard_manager->on_key_press(event);
}
void Keyboard::keyboard_release(const QKeyEvent& event){
//    m_keyboard_manager->on_key_release(event);
}







}
}
