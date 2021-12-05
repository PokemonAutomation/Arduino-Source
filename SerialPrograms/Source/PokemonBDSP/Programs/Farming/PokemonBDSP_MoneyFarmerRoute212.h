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
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MoneyFarmerRoute212_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MoneyFarmerRoute212_Descriptor();
};


class MoneyFarmerRoute212 : public SingleSwitchProgramInstance{
public:
    MoneyFarmerRoute212(const MoneyFarmerRoute212_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;
    void battle(SingleSwitchProgramEnvironment& env, uint8_t pp[4], bool man);
    void heal_and_return(ConsoleHandle& console, uint8_t pp[4]);
    void flyback_heal_and_return(ConsoleHandle& console, uint8_t pp[4]);
    static size_t total_pp(uint8_t pp[4]);

private:
    ShortcutDirection SHORTCUT;

    EnumDropdownOption START_LOCATION;

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
