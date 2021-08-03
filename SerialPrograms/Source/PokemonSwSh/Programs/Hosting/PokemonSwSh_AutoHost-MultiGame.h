/*  Multi-Game Auto-Host
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoHostMultiGame_H
#define PokemonAutomation_PokemonSwSh_AutoHostMultiGame_H

#include "Common/PokemonSwSh/PokemonSettings.h"
#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/RandomCode.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/MultiHostTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class AutoHostMultiGame_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    AutoHostMultiGame_Descriptor();
};



class AutoHostMultiGame : public SingleSwitchProgramInstance{
public:
    AutoHostMultiGame(const AutoHostMultiGame_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    void run_autohost(
        SingleSwitchProgramEnvironment& env,
        const MultiHostTable::GameSlot& game,
        uint8_t accept_FR_slot,
        uint16_t lobby_wait_delay,
        Catchability catchability
    ) const;

private:
    StartInGripOrClosed START_IN_GRIP_MENU;

    RandomCode RAID_CODE;
    BooleanCheckBox HOST_ONLINE;
    TimeExpression<uint16_t> LOBBY_WAIT_DELAY;
    MultiHostTable GAME_LIST;
    SimpleInteger<uint8_t> FR_FORWARD_ACCEPT;
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
