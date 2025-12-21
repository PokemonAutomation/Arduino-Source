/*  Donut Options Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutOptionsTest_H
#define PokemonAutomation_PokemonLZA_DonutOptionsTest_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonLZA/Options/PokemonLZA_DonutBerriesOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class DonutOptionsTest_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DonutOptionsTest_Descriptor();
};

class DonutOptionsTest : public SingleSwitchProgramInstance{
public:
    DonutOptionsTest();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    DonutBerriesTable BERRIES;
    SimpleIntegerOption<uint8_t> NUM_POWER_REQUIRED;
    FlavorPowerTable FLAVOR_POWERS;
    SimpleIntegerOption<uint8_t> NUM_DONUTS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
