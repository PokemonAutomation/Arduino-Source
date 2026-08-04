/*  Controller Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerSettings_H
#define PokemonAutomation_Controllers_ControllerSettings_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{


class ControllerSettings{
public:
    static ControllerSettings& instance(){
        static ControllerSettings settings;
        return settings;
    }

private:
    ControllerSettings()
        : COMMAND_QUEUE_LIMIT(
            "<b>Maximum Command Queue Size:</b><br>"
            "Do not queue more than this many commands to the controller at once. "
            "Larger values will tolerate longer connection interrupts, but may increase cancellation latency after a burst of commands.",
            LockMode::LOCK_WHILE_RUNNING,
            64, 4, 255
        )
        , DEVICE_LOGGING_FLAG(
            "<b>Configure Device-Specific Debug Logging:</b>",
            LockMode::LOCK_WHILE_RUNNING,
            0
        )
    {}

public:
    SimpleIntegerOption<uint8_t> COMMAND_QUEUE_LIMIT;
    SimpleIntegerOption<uint32_t> DEVICE_LOGGING_FLAG;
};


}
#endif
