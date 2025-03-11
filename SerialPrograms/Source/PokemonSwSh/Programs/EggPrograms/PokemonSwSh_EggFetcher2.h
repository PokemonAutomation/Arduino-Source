/*  Egg Fetcher 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggFetcher2_H
#define PokemonAutomation_PokemonSwSh_EggFetcher2_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
//#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggFetcher2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggFetcher2_Descriptor();
};



class EggFetcher2 : public SingleSwitchProgramInstance{
public:
    EggFetcher2();

    void run_eggfetcher(Logger& logger, ProControllerContext& context, bool deposit_automatically, uint16_t attempts) const;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint16_t> MAX_FETCH_ATTEMPTS;

    EventNotificationsOption NOTIFICATIONS;
};



}
}
}
#endif
