/*  Console State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ConsoleState_H
#define PokemonAutomation_NintendoSwitch_ConsoleState_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"

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
const std::string& ConsoleType_strings(ConsoleType type);

inline bool is_switch1(ConsoleType type){
    return type == ConsoleType::Switch1;
}
inline bool is_switch2(ConsoleType type){
    return type == ConsoleType::Switch2_Unknown
        || type == ConsoleType::Switch2_FW19_International
        || type == ConsoleType::Switch2_FW19_JapanLocked
        || type == ConsoleType::Switch2_FW20_International
        || type == ConsoleType::Switch2_FW20_JapanLocked;
}
void check_for_conflict(
    Logger& logger,
    ConsoleType user_type,
    ConsoleType detected_type
);




class ConsoleState{
public:
    ~ConsoleState();
    ConsoleState();


    bool console_type_confirmed() const;
    ConsoleType console_type() const;
    void set_console_type_user(ConsoleType type);
    void set_console_type(Logger& logger, ConsoleType type);

    bool text_size_ok() const;
    void set_text_size_ok(bool ok);

private:
    struct Data;
    Pimpl<Data> m_data;
};




}
}
#endif
