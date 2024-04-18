/*  Auto Item Printer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_AutoItemPrinter_H
#define PokemonAutomation_PokemonSV_AutoItemPrinter_H

#include <array>
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void item_printer_start_print(
    ConsoleHandle& console, BotBaseContext& context, uint8_t jobs
);
std::array<std::string, 10> item_printer_finish_print(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language
);



class AutoItemPrinter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoItemPrinter_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};



class AutoItemPrinter : public SingleSwitchProgramInstance{
public:
    AutoItemPrinter();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint16_t> NUM_ROUNDS;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void enter_printing_mode(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
};



}
}
}
#endif
