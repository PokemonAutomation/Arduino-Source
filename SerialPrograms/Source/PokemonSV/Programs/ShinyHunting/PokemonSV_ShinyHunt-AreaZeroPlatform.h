/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H
#define PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H

//#include <functional>
//#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldSensors.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_AreaZeroPlatform.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class LetsGoHpWatcher;
class DiscontiguousTimeTracker;
class LetsGoEncounterBotTracker;



class ShinyHuntAreaZeroPlatform_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAreaZeroPlatform_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAreaZeroPlatform : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~ShinyHuntAreaZeroPlatform();
    ShinyHuntAreaZeroPlatform();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual std::string check_validity() const override;
    virtual void on_config_value_changed(void* object) override;

    enum class Location{
        UNKNOWN,
        ZERO_GATE_FLY_SPOT,
        TRAVELING_TO_PLATFORM,
        AREA_ZERO,
    };
//    enum class State{
//        TRAVERSAL,
//        INSIDE_GATE_AND_RETURN,
//        LEAVE_AND_RETURN,
//        RESET_AND_RETURN,
//        RESET_SANDWICH,
//    };

    void set_flags(SingleSwitchProgramEnvironment& env);
    void run_state(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context
    );
    void set_flags_and_run_state(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context
    );

    //  Returns true on success.
    bool run_traversal(ProControllerContext& context);


private:
    OCR::LanguageOCROption LANGUAGE;

    enum class Mode{
        START_ON_PLATFORM,
        START_AT_ZERO_GATE_FLY_SPOT,
        MAKE_SANDWICH,
    };
    EnumDropdownOption<Mode> MODE;

    BooleanCheckBoxOption FLYING_UNLOCKED;

    enum class Path{
        PATH0,
        PATH1,
        PATH2,
    };
    EnumDropdownOption<Path> PATH0;

    SimpleIntegerOption<uint16_t> SANDWICH_RESET_IN_MINUTES;
    SandwichMakerOption SANDWICH_OPTIONS;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PlatformResetSettings PLATFORM_RESET;
    NavigatePlatformSettings NAVIGATE_TO_PLATFORM;
    FloatingPointOption AUTO_HEAL_PERCENT;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SingleSwitchProgramEnvironment* m_env;

    OverworldSensors* m_sensors;
//    LetsGoHpWatcher* m_hp_watcher;
    DiscontiguousTimeTracker* m_time_tracker;
    LetsGoEncounterBotTracker* m_encounter_tracker;

    uint64_t m_iterations = 0;
    Location m_current_location;
    Location m_saved_location;
//    State m_state;

    //  Set to true if we should save on the first available opportunity.
    bool m_pending_save;
    bool m_pending_platform_reset;
    bool m_pending_sandwich;
    bool m_reset_on_next_sandwich;

//    enum class SavedLocation{
//        NONE,
//        ZERO_GATE_FLY_SPOT,
//        AREA_ZERO,
//    };
//    SavedLocation m_last_save;

    WallClock m_last_sandwich;

    size_t m_consecutive_failures;
};





}
}
}
#endif
