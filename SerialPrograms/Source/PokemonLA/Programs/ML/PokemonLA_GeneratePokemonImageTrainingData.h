/*  Generate Pokemon Image Training Data
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Starting at Jubilife Village or any camp, talk to npc to get pokemon from box to party,
 *  throw the pokemon out onto the field to take screenshots with it, move around it to take
 *  screenshots from various angles. Repeat for rest of the pokemon in specified boxes.
 */

#ifndef PokemonAutomation_PokemonLA_GeneratePokemonImageTrainingData_H
#define PokemonAutomation_PokemonLA_GeneratePokemonImageTrainingData_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class GeneratePokemonImageTrainingData_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GeneratePokemonImageTrainingData_Descriptor();
};


class GeneratePokemonImageTrainingData : public SingleSwitchProgramInstance{
public:
    GeneratePokemonImageTrainingData();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:

    SimpleIntegerOption<uint16_t> NUM_BOXES;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif