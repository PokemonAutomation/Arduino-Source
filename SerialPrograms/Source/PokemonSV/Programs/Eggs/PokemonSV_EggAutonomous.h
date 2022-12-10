/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_EggAutonomous_H
#define PokemonAutomation_PokemonSV_EggAutonomous_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_EggHatchFilter.h"

// #include <functional>

namespace PokemonAutomation{

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
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    int fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    bool hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int num_eggs_in_party);

    void collect_eggs_at_picnic(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void reset_position_to_flying_spot(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    int picnic_party_to_hatch_party(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void move_circles_to_hatch_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int num_eggs_in_party);

    uint8_t read_party_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void open_map(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void fly_to_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void enter_box_system(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void leave_box_system(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void save_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool from_overworld);

    void reset_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, const std::string& error_msg);

    void handle_recoverable_error(SingleSwitchProgramEnvironment& env, BotBaseContext& context, OperationFailedException& e, size_t& consecutive_failures);

    void dump_unrecoverable_error(SingleSwitchProgramEnvironment& env, const std::string& error_name);

    // void call_with_debug_dump(SingleSwitchProgramEnvironment& env, BotBaseContext& context, std::function<void())

private:
    // Will need this to preserve raid den
    // TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint64_t> MAX_NUM_SANDWICHES;

    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint8_t> MAX_KEEPERS;

    enum class AutoSave{
        NoAutoSave,
        AfterStartAndKeep,
        EveryBatch,
    };
    EnumDropdownOption<AutoSave> AUTO_SAVING;

    Pokemon::EggHatchFilterTable FILTERS;

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

    // Whether we have an error that is recoverable
    // In most error cases, the program will try soft resetting to recover from it.
    // But if there is error during saving the game, or resetting the game,
    // we don't recover from that and instead stop the program to let user handle it.
    // So the code needs a flag to know whether an error occured during those cases.
    bool m_error_recoverable = true;
};




}
}
}
#endif
