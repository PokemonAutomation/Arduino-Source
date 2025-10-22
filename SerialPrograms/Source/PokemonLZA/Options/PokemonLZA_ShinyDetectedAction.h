/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLZA_ShinyDetectedAction_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options//TimeDurationOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLZA{


enum class ShinyDetectedActionType{
    IGNORE,
    STOP_PROGRAM,
    STOP_AFTER_COUNT,
};


class ShinyDetectedActionOption : public GroupOption, public ConfigOption::Listener{
public:
    ~ShinyDetectedActionOption();
    ShinyDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay,
        ShinyDetectedActionType default_action = ShinyDetectedActionType::STOP_AFTER_COUNT
    );

    bool stop_on_shiny(uint8_t current_count) const;

    virtual void on_config_value_changed(void* object) override;

    StaticTextOption DESCRIPTION;
    EnumDropdownOption<ShinyDetectedActionType> ACTION;
    SimpleIntegerOption<uint8_t> MAX_COUNT;
    BooleanCheckBoxOption TAKE_VIDEO;
    MillisecondsOption SCREENSHOT_DELAY;
    EventNotificationOption NOTIFICATIONS;
};



bool on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    ShinyDetectedActionOption& options,
    uint8_t current_count,
    float error_coefficient
);




}
}
}
#endif
