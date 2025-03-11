/*  Flying Trial Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_FlyingTrialFarmer_H
#define PokemonAutomation_PokemonSV_FlyingTrialFarmer_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class FlyingTrialFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FlyingTrialFarmer_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};




class FlyingTrialFarmer : public SingleSwitchProgramInstance{
public:
    FlyingTrialFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint16_t> NUM_TRIALS;
    SimpleIntegerOption<uint16_t> SAVE_NUM_ROUNDS;

    enum class FlightPath{
        FRONT_ENTRY,
        BACK_ENTRY_STRAIGHT,
        BACK_ENTRY_SOFT_TURN,
        BACK_ENTRY_HARD_TURN
    };
    EnumDropdownOption<FlightPath> FLIGHT_PATH;

    BooleanCheckBoxOption INVERT_CONTROLS_WHILE_FLYING;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    bool run_rewards(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    uint8_t get_final_y_axis(int8_t delta_y);
};



}
}
}
#endif
