/*  Raid Item Farmer (OHKO)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H
#define PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H

//#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"

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
    TimeExpressionOption<uint16_t> WAIT_FOR_STAMP_DELAY;
    TimeExpressionOption<uint16_t> ENTER_STAMP_MASH_DURATION;
    TimeExpressionOption<uint16_t> RAID_START_MASH_DURATION;
    TimeExpressionOption<uint16_t> RAID_START_TO_ATTACK_DELAY;
    TimeExpressionOption<uint16_t> ATTACK_TO_CATCH_DELAY;
    TimeExpressionOption<uint16_t> RETURN_TO_OVERWORLD_DELAY;
    TimeExpressionOption<uint32_t> TOUCH_DATE_INTERVAL;
};




}
}
}
#endif
