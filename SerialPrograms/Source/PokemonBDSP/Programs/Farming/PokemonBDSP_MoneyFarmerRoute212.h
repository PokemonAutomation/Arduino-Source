/*  Money Farmer (Route 212)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MoneyFarmerRoute212_H
#define PokemonAutomation_PokemonBDSP_MoneyFarmerRoute212_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MoneyFarmerRoute212_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MoneyFarmerRoute212_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class MoneyFarmerRoute212 : public SingleSwitchProgramInstance{
public:
    MoneyFarmerRoute212();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    // Run the battle loop. Return true if the program should stop.
    bool battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint8_t pp[4], bool man);
    // From the row above the old couple, heal Pokemon and return.
    // Return true if VS Seeker needs charging.
    bool heal_after_battle_and_return(ConsoleHandle& console, BotBaseContext& context, uint8_t pp[4]);
    // Starting in front of the Hearthome Pokecenter, heal and return
    // to the old couple.
    void heal_at_center_and_return(ConsoleHandle& console, BotBaseContext& context, uint8_t pp[4]);
    // Fly from the old couple to Hearthome Pokecenter, heal and return.
    void fly_to_center_heal_and_return(ConsoleHandle& console, BotBaseContext& context, uint8_t pp[4]);
    // Move around to charge VS Seeker.
    void charge_vs_seeker(BotBaseContext& context);

    static size_t total_pp(uint8_t pp[4]);

private:
    ShortcutDirection SHORTCUT;

    EnumDropdownOption START_LOCATION;

    EnumDropdownOption HEALING_METHOD;

    EnumDropdownOption ON_LEARN_MOVE;

    SimpleIntegerOption<uint8_t> MOVE1_PP;
    SimpleIntegerOption<uint8_t> MOVE2_PP;
    SimpleIntegerOption<uint8_t> MOVE3_PP;
    SimpleIntegerOption<uint8_t> MOVE4_PP;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
