/*  RNG Manipulation of the Highlight Watt Trader in the Snowslide Slope area in the Crown Tundra
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Based on Anubis' findings: https://docs.google.com/spreadsheets/u/0/d/1pNYtCJKRh_efX9LvzjCiA-0n2lGSFnVmSWwmPzgSOMw/htmlview
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_DailyHighlightRNG_H
#define PokemonAutomation_PokemonSwSh_DailyHighlightRNG_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Options/StringSelectTableOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_Xoroshiro128Plus.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_DailyHighlightDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DailyHighlightRNG_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DailyHighlightRNG_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class DailyHighlightRNG : public SingleSwitchProgramInstance{
public:
    DailyHighlightRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint16_t> NUM_HIGHLIGHTS;
    BooleanCheckBoxOption CONTINUE;
    StringSelectTableOption HIGHLIGHT_SELECTION;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint32_t> MAX_UNKNOWN_ADVANCES;
    SimpleIntegerOption<uint16_t> ADVANCE_PRESS_DURATION;
    SimpleIntegerOption<uint16_t> ADVANCE_RELEASE_DURATION;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;

    void move_to_trader(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    uint8_t calibrate_num_npc_from_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context, Pokemon::Xoroshiro128Plus& rng);
    void navigate_to_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    size_t calculate_target(SingleSwitchProgramEnvironment& env, Pokemon::Xoroshiro128PlusState state, uint8_t num_npcs, std::vector<std::string> wanted_highlights);
    void leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool buy_highlight);
    void recover_from_wrong_state(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void advance_date(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint8_t& year);
};




}
}
}
#endif



