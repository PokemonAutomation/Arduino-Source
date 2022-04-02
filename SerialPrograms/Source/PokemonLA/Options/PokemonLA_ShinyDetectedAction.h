/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLA_ShinyDetectedAction_H

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Options/BatchOption/GroupOption.h"
#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
    class BotBaseContext;
    class EventNotificationOption;
    class StatsTracker;
    class ProgramEnvironment;
    class ConsoleHandle;
    class AsyncCommandSession;
namespace NintendoSwitch{
namespace PokemonLA{

class ShinySoundDetector;


class ShinyStatIncrementer{
public:
    virtual void add_shiny() = 0;
};

struct ShinySoundResults{
    float error_coefficient;
    QImage screenshot;
};




#if 0
//  Throw when you want to force the program to stop.
class ShinyDetectedException : public OperationCancelledException{
public:
    ShinyDetectedException(QImage image)
        : m_screenshot(std::move(image))
    {}
    virtual const char* name() const override{ return "ShinyDetectedException"; }
    QImage consume_screenshot(){
        return m_screenshot;
    }

private:
    QImage m_screenshot;
};
#endif


enum class ShinyDetectedAction{
    IGNORE,
    STOP_PROGRAM,
    TAKE_VIDEO_STOP_PROGRAM,
};


class ShinyDetectedActionOption : public GroupOption{
public:
    ShinyDetectedActionOption(QString default_delay_ticks = "0 * TICKS_PER_SECOND");

    bool stop_on_shiny() const;

    StaticTextOption DESCRIPTION;
    EnumDropdownOption ACTION;
//    BooleanCheckBoxOption STOP_PROGRAM;
//    BooleanCheckBoxOption TAKE_VIDEO;
    TimeExpressionOption<uint16_t> SCREENSHOT_DELAY;
    EventNotificationOption NOTIFICATIONS;
};


#if 0
//  Return true if program should stop.
bool run_on_shiny(
    BotBaseContext& context,
    const ShinyDetectedActionOption& option
);

bool run_on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    AsyncCommandSession& command_session,
    ShinyDetectedActionOption& option,
    const StatsTracker* session_stats
);
#endif


void on_shiny_sound(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    ShinyDetectedActionOption& options,
    const ShinySoundResults& results
);



}
}
}
#endif
