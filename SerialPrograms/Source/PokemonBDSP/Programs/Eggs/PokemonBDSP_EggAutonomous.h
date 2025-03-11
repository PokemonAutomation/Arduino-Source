/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggAutonomous_H
#define PokemonAutomation_PokemonBDSP_EggAutonomous_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonBDSP_EggAutonomousState.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class EggAutonomous_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggAutonomous_Descriptor();
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class EggAutonomous : public SingleSwitchProgramInstance{
public:
    EggAutonomous();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_batch(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        EggAutonomousState& saved_state,
        EggAutonomousState& current_state
    );

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    ShortcutDirectionOption SHORTCUT;
    SimpleIntegerOption<uint8_t> MAX_KEEPERS;
    MillisecondsOption TRAVEL_TIME_PER_FETCH0;
    IntegerEnumDropdownOption NUM_EGGS_IN_COLUMN;

    enum class AutoSave{
        NoAutoSave,
        AfterStartAndKeep,
        EveryBatch,
    };
    EnumDropdownOption<AutoSave> AUTO_SAVING;
    
    Pokemon::StatsHuntIvJudgeFilterTable FILTERS0;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY_KEEP;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption SCROLL_TO_READ_DELAY0;
};



}
}
}
#endif
