/*  Raid Item Farmer (OHKO)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H
#define PokemonAutomation_PokemonSwSh_RaidItemFarmerOHKO_H

//#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class RaidItemFarmerOHKO : public MultiSwitchProgram{
public:
    RaidItemFarmerOHKO();

    virtual void program(MultiSwitchProgramEnvironment& env) const override;

private:
    BooleanCheckBox BACKUP_SAVE;
//    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> WAIT_FOR_STAMP_DELAY;
    TimeExpression<uint16_t> ENTER_STAMP_MASH_DURATION;
    TimeExpression<uint16_t> RAID_START_MASH_DURATION;
    TimeExpression<uint16_t> RAID_START_TO_ATTACK_DELAY;
    TimeExpression<uint16_t> ATTACK_TO_CATCH_DELAY;
    TimeExpression<uint16_t> RETURN_TO_OVERWORLD_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};




}
}
}
#endif
