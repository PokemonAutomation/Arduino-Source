/*  Rolling Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostRolling_H
#define PokemonAutomation_PokemonSwSh_AutoHostRolling_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/RandomCode.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/Catchability.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostRolling_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AutoHostRolling_Descriptor();
};



class AutoHostRolling : public SingleSwitchProgramInstance{
public:
    AutoHostRolling(const AutoHostRolling_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    RandomCode RAID_CODE;
    SimpleInteger<uint8_t> SKIPS;
    BooleanCheckBox BACKUP_SAVE;
    BooleanCheckBox HOST_ONLINE;
    TimeExpression<uint16_t> LOBBY_WAIT_DELAY;
    CatchabilitySelector CATCHABILITY;
    SimpleInteger<uint8_t> FRIEND_ACCEPT_USER_SLOT;
    TimeExpression<uint16_t> EXTRA_DELAY_BETWEEN_RAIDS;

    SimpleInteger<uint8_t> MOVE_SLOT;
    BooleanCheckBox DYNAMAX;
    SimpleInteger<uint8_t> TROLL_HOSTING;

    BooleanCheckBox ALTERNATE_GAMES;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;

    SectionDivider m_internet_settings;
    TimeExpression<uint16_t> CONNECT_TO_INTERNET_DELAY;
    TimeExpression<uint16_t> ENTER_ONLINE_DEN_DELAY;
    TimeExpression<uint16_t> OPEN_ONLINE_DEN_LOBBY_DELAY;
    TimeExpression<uint16_t> RAID_START_TO_EXIT_DELAY;
    TimeExpression<uint16_t> DELAY_TO_SELECT_MOVE;
};



}
}
}
#endif

