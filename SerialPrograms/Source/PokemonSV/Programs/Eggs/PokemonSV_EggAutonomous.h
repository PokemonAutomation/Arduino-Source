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
    void fetch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    size_t hatch_eggs_full_routine(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void from_pokecenter_get_meal(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void collect_eggs_at_pokemon_league(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void open_map(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void fly_to_overworld(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void save_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, bool from_overworld);

    void reset_game(SingleSwitchProgramEnvironment& env, BotBaseContext& context, const std::string& error_msg);

    void handle_recoverable_error(SingleSwitchProgramEnvironment& env, BotBaseContext& context, OperationFailedException& e, size_t& consecutive_failures);

private:
    // Will need this to preserve raid den
    // TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    SimpleIntegerOption<uint64_t> BUDGET;

    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint8_t> MAX_KEEPERS;

    enum class AutoSave{
        NoAutoSave,
        AfterStartAndKeep,
        EveryBatch,
    };
    EnumDropdownOption<AutoSave> AUTO_SAVING;

    SimpleIntegerOption<uint8_t> MAX_NUM_BOXES;

    Pokemon::EggHatchFilterTable FILTERS;

    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY_KEEP;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption m_notification_noop;
    EventNotificationsOption NOTIFICATIONS;

    // We need to keep track of when saving the game, where the player is.
    // This is needed so that if we reset game, we know where we are
    bool m_save_location_at_restaurant_town = true;

    // How many pokemon have been kept so far
    uint8_t m_num_kept = 0;
    // How many money spent so far
    size_t m_money_spent = 0;

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
