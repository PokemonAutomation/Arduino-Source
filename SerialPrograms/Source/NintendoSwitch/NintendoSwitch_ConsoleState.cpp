/*  Console State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "NintendoSwitch_ConsoleState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



const std::string& ConsoleType_strings(ConsoleType type){
    static std::string STRINGS[] = {
        "Unknown",
        "Switch 1 + OLED",
        "Switch 2 (unknown model)",
        "Switch 2 (FW19, international)",
        "Switch 2 (FW19, Japan-locked)",
        "Switch 2 (FW20, international)",
        "Switch 2 (FW20, Japan-locked)",
    };
    size_t index = (size_t)type;
    if (index < sizeof(STRINGS) / sizeof(std::string)){
        return STRINGS[(size_t)type];
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Invalid ConsoleType enum: " + std::to_string(index)
    );
}

[[noreturn]] void throw_conflict(
    Logger& logger,
    ConsoleType user_type,
    ConsoleType detected_type
){
    throw UserSetupError(
        logger,
        std::string("Conflicting Console Types:") +
        "\n    User Selected: " + ConsoleType_strings(user_type) +
        "\n    Detected Type: " + ConsoleType_strings(detected_type) +
        "\n\nIf you think this is a bug, please report it to either our Github or our Discord server!"
    );
}



void check_for_conflict(
    Logger& logger,
    ConsoleType user_type,
    ConsoleType detected_type
){
    if (user_type == detected_type){
        return;
    }
    if (user_type == ConsoleType::Unknown || detected_type == ConsoleType::Unknown){
        return;
    }

    if (is_switch1(user_type) && is_switch1(detected_type)){
        return;
    }

    if (is_switch2(user_type) && is_switch2(detected_type)){
        if (user_type == ConsoleType::Switch2_Unknown || detected_type == ConsoleType::Switch2_Unknown){
            return;
        }
        if (user_type != detected_type){
            throw_conflict(logger, user_type, detected_type);
        }
        return;
    }

    throw_conflict(logger, user_type, detected_type);
}






struct ConsoleState::Data{
    Data()
        : m_console_type_confirmed(false)
        , m_console_type(ConsoleType::Unknown)
        , m_text_size_standard(false)
    {}

    std::atomic<bool> m_console_type_confirmed;
    std::atomic<ConsoleType> m_console_type;

    std::atomic<bool> m_text_size_standard;
};



ConsoleState::~ConsoleState() = default;
ConsoleState::ConsoleState()
    : m_data(CONSTRUCT_TOKEN)
{}

bool ConsoleState::console_type_confirmed() const{
    return m_data->m_console_type_confirmed.load(std::memory_order_relaxed);
}
ConsoleType ConsoleState::console_type() const{
    return m_data->m_console_type.load(std::memory_order_relaxed);
}
void ConsoleState::set_console_type_user(ConsoleType type){
    m_data->m_console_type_confirmed.store(false, std::memory_order_relaxed);
    m_data->m_console_type.store(type, std::memory_order_relaxed);
}
void ConsoleState::set_console_type(Logger& logger, ConsoleType type){
    ConsoleType current = console_type();
    bool confirmed = m_data->m_console_type_confirmed.load(std::memory_order_relaxed);
    if (!confirmed){
        check_for_conflict(logger, current, type);
    }

    do{
        if (type == ConsoleType::Unknown){
            return;
        }
        if (current == ConsoleType::Unknown){
            break;
        }

        if (type == ConsoleType::Switch2_Unknown){
            return;
        }

    }while (false);

    if (!confirmed || current != type){
        logger.log(
            std::string("Setting console type to: ") + ConsoleType_strings(type),
            COLOR_BLUE
        );
    }

    m_data->m_console_type.store(type, std::memory_order_relaxed);
    m_data->m_console_type_confirmed.store(true, std::memory_order_relaxed);
}



bool ConsoleState::text_size_ok() const{
    return m_data->m_text_size_standard.load(std::memory_order_relaxed);
}
void ConsoleState::set_text_size_ok(bool ok){
    m_data->m_text_size_standard.store(ok, std::memory_order_relaxed);
}
























}
}
