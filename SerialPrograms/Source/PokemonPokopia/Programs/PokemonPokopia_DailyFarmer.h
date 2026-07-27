/*  Daily Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_DailyFarmer_H
#define PokemonAutomation_PokemonPokopia_DailyFarmer_H

#include "Common/Cpp/DateTime.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonPokopia{


class DailyFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DailyFarmer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

enum class DateSkipMode{
    NEXT_FRIDAY,
    TOMMORROW
};

class DailyFarmer : public SingleSwitchProgramInstance{
public:
    DailyFarmer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Go from in game to the date change menu
    void go_to_date_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Go to date menu and change the switch date and return to the game
    // Either skip to the next Friday or skip to the next day depending on the DateSkipMode
    void date_skip(SingleSwitchProgramEnvironment& env, ProControllerContext& context, DateSkipMode mode);

    // Add today's stamp to the stamp collection
    // Exit the PC when done
    void add_todays_stamp(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // // Open the shop menu from the main PC menu and buy all available recipes until the spend limit reached
    // // Exit the PC when done
    // // Return true if the spend limit is reached and the program should stop buying recipes
    // bool buy_recipes(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    DeferredStopButtonOption STOP_AFTER_CURRENT;
    SimpleIntegerOption<uint32_t> NUM_SKIPS;
    BooleanCheckBoxOption COLLECT_STAMPS;
    // BooleanCheckBoxOption BUY_RECIPES;
    // SimpleIntegerOption<uint32_t> SPEND_LIMIT;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
