/*  Cram-o-matic RNG Manipulation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_CramomaticRNG_H
#define PokemonAutomation_PokemonSwSh_CramomaticRNG_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Pokemon_Xoroshiro128Plus.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_CramomaticTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class CramomaticRNG_Descriptor : public SingleSwitchProgramDescriptor{
public:
    CramomaticRNG_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

struct CramomaticTarget{
    CramomaticBallType ball_type;
    bool is_bonus;
    size_t needed_advances;
};

class CramomaticRNG : public SingleSwitchProgramInstance{
public:
    CramomaticRNG();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint32_t> NUM_APRICORN_ONE;
    SimpleIntegerOption<uint32_t> NUM_APRICORN_TWO;
    SimpleIntegerOption<uint32_t> NUM_NPCS;
    CramomaticTable BALL_TABLE;

    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    SimpleIntegerOption<uint16_t> MAX_PRIORITY_ADVANCES;
    SimpleIntegerOption<uint16_t> MAX_UNKNOWN_ADVANCES;
    MillisecondsOption ADVANCE_PRESS_DURATION;
    MillisecondsOption ADVANCE_RELEASE_DURATION;
    BooleanCheckBoxOption SAVE_SCREENSHOTS;
    BooleanCheckBoxOption LOG_VALUES;

    void navigate_to_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    CramomaticTarget calculate_target(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, std::vector<CramomaticSelection> wanted_balls);
    void leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void choose_apricorn(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool sport);
    std::pair<bool, std::string> receive_ball(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void recover_from_wrong_state(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};




}
}
}
#endif



