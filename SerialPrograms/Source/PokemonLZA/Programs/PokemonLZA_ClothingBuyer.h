/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ClothingBuyer_H
#define PokemonAutomation_PokemonLZA_ClothingBuyer_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class ClothingBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};

class ClothingBuyer : public SingleSwitchProgramInstance{
public:
    ClothingBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint8_t> NUM_CATEGORY;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
