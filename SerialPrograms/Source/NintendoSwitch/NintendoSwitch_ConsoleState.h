/*  Console State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleState_H
#define PokemonAutomation_NintendoSwitch_ConsoleState_H

#include <atomic>

namespace PokemonAutomation{
namespace NintendoSwitch{



enum class ConsoleType{
    Unknown,
    Switch1,
    Switch2_Unknown,
    Switch2_FW19_International,
    Switch2_FW19_JapanLocked,
    Switch2_FW20_International,
    Switch2_FW20_JapanLocked,
};
inline bool is_switch2(ConsoleType type){
    return type == ConsoleType::Switch2_Unknown
        || type == ConsoleType::Switch2_FW19_International
        || type == ConsoleType::Switch2_FW19_JapanLocked
        || type == ConsoleType::Switch2_FW20_International
        || type == ConsoleType::Switch2_FW20_JapanLocked;
}






class ConsoleState{
public:
    ConsoleState()
        : m_console_type(ConsoleType::Unknown)
        , m_text_size_standard(false)
    {}

    ConsoleType console_type() const{
        return m_console_type.load(std::memory_order_relaxed);
    }
    bool text_size_ok() const{
        return m_text_size_standard.load(std::memory_order_relaxed);
    }


    void set_console_type(ConsoleType type){
        m_console_type.store(type, std::memory_order_relaxed);
    }
    void set_text_size_ok(bool ok){
        m_text_size_standard.store(ok, std::memory_order_relaxed);
    }

private:
    std::atomic<ConsoleType> m_console_type;
    std::atomic<bool> m_text_size_standard;
};




}
}
#endif
