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
//#include "CommonFramework/Options/BooleanCheckBoxOption.h"
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



enum class ShinyDetectedAction{
    IGNORE,
    STOP_PROGRAM,
    TAKE_VIDEO_STOP_PROGRAM,
};


class ShinyDetectedActionOption : public GroupOption{
public:
    ShinyDetectedActionOption();

    bool stop_on_shiny() const;
    bool do_nothing() const;

//    BooleanCheckBoxOption STOP_PROGRAM;
//    BooleanCheckBoxOption TAKE_VIDEO;
    StaticTextOption DESCRIPTION;
    EnumDropdownOption ACTION;
    TimeExpressionOption<uint16_t> VIDEO_DELAY;
    EventNotificationOption NOTIFICATIONS;
};


#if 0
//  Return true if program should stop.
bool run_on_shiny(
    const BotBaseContext& context,
    const ShinyDetectedActionOption& option
);

bool run_on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    AsyncCommandSession& command_session,
    ShinyDetectedActionOption& option,
    const StatsTracker* session_stats
);
#endif


void on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options, QImage screenshot
);



}
}
}
#endif
