/*  Berry Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BerryBuyer_H
#define PokemonAutomation_PokemonLZA_BerryBuyer_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class BerryBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BerryBuyer_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BerryBuyer : public SingleSwitchProgramInstance{
public:
    BerryBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    enum class BerryType{
        POMEG,
        KELPSY,
        QUALOT,
        HONDEW,
        GREPA,
        TAMATO,
    };
    EnumDropdownOption<BerryType> BERRY_TYPE;
    SimpleIntegerOption<uint16_t> NUM_PURCHASE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
