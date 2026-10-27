/*  Clothing Buyer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ClothingBuyer_H
#define PokemonAutomation_PokemonSV_ClothingBuyer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class ClothingBuyer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ClothingBuyer_Descriptor();
};

class ClothingBuyer : public SingleSwitchProgramInstance{
public:
    using Descriptor = ClothingBuyer_Descriptor;
    ClothingBuyer();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    BooleanCheckBoxOption USE_LP;
    BooleanCheckBoxOption WEAR_NEW_CLOTHES;
    SimpleIntegerOption<uint8_t> NUM_CATEGORY;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif



