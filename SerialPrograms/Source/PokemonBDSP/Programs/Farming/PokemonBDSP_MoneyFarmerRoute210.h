/*  Money Farmer (Route 210)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MoneyFarmerRoute210_H
#define PokemonAutomation_PokemonBDSP_MoneyFarmerRoute210_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MoneyFarmerRoute210_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    MoneyFarmerRoute210_Descriptor();
};


class MoneyFarmerRoute210 : public SingleSwitchProgramInstance{
public:
    MoneyFarmerRoute210(const MoneyFarmerRoute210_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;
    void battle(SingleSwitchProgramEnvironment& env, uint8_t pp0[4], uint8_t pp1[4]);
    void heal_and_return(ConsoleHandle& console, uint8_t pp0[4], uint8_t pp1[4]);
    void flyback_heal_and_return(ConsoleHandle& console, uint8_t pp0[4], uint8_t pp1[4]);
    static bool has_pp(uint8_t pp0[4], uint8_t pp1[4]);

private:
    ShortcutDirection SHORTCUT;

    SimpleIntegerOption<uint8_t> MON0_MOVE1_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE2_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE3_PP;
    SimpleIntegerOption<uint8_t> MON0_MOVE4_PP;

    SimpleIntegerOption<uint8_t> MON1_MOVE1_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE2_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE3_PP;
    SimpleIntegerOption<uint8_t> MON1_MOVE4_PP;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
