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
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonTools/Options/StringSelectTableOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint16_t> NUM_HIGHLIGHTS;
    BooleanCheckBoxOption FIX_TIME_WHEN_DONE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    SimpleIntegerOption<uint16_t> SAVE_INTERVAL;
    SimpleIntegerOption<uint16_t> CALIBRATION_INTERAVAL;
    StringSelectTableOption HIGHLIGHT_SELECTION;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint8_t> MAX_EXPECTED_NPCS;
    SimpleIntegerOption<uint8_t> CALIBRATION_REPEATS;
    SimpleIntegerOption<uint8_t> CALIBRATION_THRESHOLD;
    SimpleIntegerOption<uint32_t> MAX_UNKNOWN_ADVANCES;
    MillisecondsOption ADVANCE_PRESS_DURATION;
    MillisecondsOption ADVANCE_RELEASE_DURATION;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;

    void move_to_trader(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void interact_with_trader(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void return_to_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool wait_after_detection = true);
    void buy_highlight(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    uint8_t calibrate_num_npc_from_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Pokemon::Xoroshiro128Plus& rng);
    void navigate_to_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    size_t calculate_target(SingleSwitchProgramEnvironment& env, Pokemon::Xoroshiro128PlusState state, uint8_t num_npcs, std::vector<std::string> wanted_highlights);
    void prepare_game_state(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void advance_date(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t& year);
};




}
}
}
#endif



