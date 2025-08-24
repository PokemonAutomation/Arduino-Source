/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggAutonomous_H
#define PokemonAutomation_PokemonSV_EggAutonomous_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonSV/Options/PokemonSV_EggPowerSandwichOption.h"

// #include <functional>

namespace PokemonAutomation{

class ScreenshotException;
class OperationFailedException;

namespace NintendoSwitch{
namespace PokemonSV{



class EggAutonomous_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggAutonomous_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;

};



// Automatically hatch eggs to farm shiny
class EggAutonomous : public SingleSwitchProgramInstance{
public:
    EggAutonomous();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    int fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int num_eggs_in_party);

    void reset_position_to_flying_spot(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    int picnic_party_to_hatch_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void process_one_baby(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t egg_index, uint8_t num_eggs_in_party);

    bool move_pokemon_to_keep(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint8_t pokemon_row_in_party);

    void save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool from_overworld);

    //  Return true if you should rethrow.
    bool handle_recoverable_error(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        EventNotificationOption& notification,
        const ScreenshotException& e,
        size_t& consecutive_failures
    );

    // void call_with_debug_dump(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::function<void())

private:
    // Will need this to preserve raid den
    // TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    enum class EggAutoLocation{
        ZeroGate,
        NorthLighthouse,
    };
    EnumDropdownOption<EggAutoLocation> LOCATION;

    SimpleIntegerOption<uint8_t> MAX_KEEPERS;

    enum class AutoSave{
        NoAutoSave,
        AfterStartAndKeep,
        EveryBatch,
        AfterFetchComplete,
    };
    EnumDropdownOption<AutoSave> AUTO_SAVING;

    EggPowerSandwichOption EGG_SANDWICH;

    BooleanCheckBoxOption HAS_CLONE_RIDE_POKEMON;

    IntegerEnumDropdownOption KEEP_BOX_LOCATION;
    
    Pokemon::StatsHuntIvJudgeFilterTable FILTERS0;

    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY_KEEP;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption m_notification_noop;
    EventNotificationsOption NOTIFICATIONS;

    // How many pokemon have been kept so far
    uint8_t m_num_kept = 0;
    // How many sandwich spent so far
    size_t m_num_sandwich_spent = 0;
    // The program first fetchs some eggs, then hatches them.
    // If user selects AUTO_SAVING to be on, then during hatching, when a recoverable error happens, we reset game.
    // If we have saved the game after fetching phase, then when reloading the game, we may still have eggs in boxes
    // that need hatching.
    // If we haven't saved after fetching, then when reloading the game, there is no eggs in boxes. We can directly
    // go to the next egg fetching phase.
    // To tell apart the two cases, we need this bool var:
    bool m_saved_after_fetched_eggs = false;
    // When we find a pokemon to keep, we don't want the game to be reset if we haven't placed a save to protect the
    // kept the pokemon. This flag is used to signal when we are in this "don't reset" stage.
    bool m_in_critical_to_save_stage = false;
};

void change_settings_egg_program(SingleSwitchProgramEnvironment& env, ProControllerContext& context,  Language language);


}
}
}
#endif
