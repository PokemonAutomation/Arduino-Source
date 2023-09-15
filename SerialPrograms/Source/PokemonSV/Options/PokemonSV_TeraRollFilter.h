/*  Tera Roll Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRollFilter_H
#define PokemonAutomation_PokemonSV_TeraRollFilter_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
    class Logger;
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class TeraRollFilter : public GroupOption{
public:
    TeraRollFilter();

    void start_program_check(Logger& logger) const;

    enum class FilterResult{
        NO_RAID,
        FAILED,
        PASSED,
    };

    FilterResult run_filter(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        std::string* pokemon_slug = nullptr, size_t* stars = nullptr
    );

    enum class EventCheckMode{
        CHECK_ALL,
        CHECK_ONLY_EVENT,
        CHECK_ONLY_NONEVENT,
    };
    EnumDropdownOption<EventCheckMode> EVENT_CHECK_MODE;

    SimpleIntegerOption<uint8_t> MIN_STARS;
    SimpleIntegerOption<uint8_t> MAX_STARS;
};



}
}
}
#endif
