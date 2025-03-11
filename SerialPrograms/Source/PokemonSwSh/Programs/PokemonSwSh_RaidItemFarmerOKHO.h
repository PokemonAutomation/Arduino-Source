/*  Raid Item Farmer (OHKO)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H
#define PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidItemFarmerOHKO_Descriptor : public MultiSwitchProgramDescriptor{
public:
    RaidItemFarmerOHKO_Descriptor();
};



class RaidItemFarmerOHKO : public MultiSwitchProgramInstance{
public:
    RaidItemFarmerOHKO();

    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    BooleanCheckBoxOption BACKUP_SAVE;
//    SectionDivider m_advanced_options;
    MillisecondsOption WAIT_FOR_STAMP_DELAY0;
    MillisecondsOption ENTER_STAMP_MASH_DURATION0;
    MillisecondsOption RAID_START_MASH_DURATION0;
    MillisecondsOption RAID_START_TO_ATTACK_DELAY0;
    MillisecondsOption ATTACK_TO_CATCH_DELAY0;
    MillisecondsOption RETURN_TO_OVERWORLD_DELAY0;
    MillisecondsOption TOUCH_DATE_INTERVAL0;
};




}
}
}
#endif
