/*  Egg Autonomous State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggAutonomousState_H
#define PokemonAutomation_PokemonBDSP_EggAutonomousState_H

#include "Common/Compiler.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


struct EggAutonomousStats : public StatsTracker{
    EggAutonomousStats();
    std::atomic<uint64_t>& m_hatched;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_fetch_attempts;
    std::atomic<uint64_t>& m_fetch_success;
    std::atomic<uint64_t>& m_shinies;
};



class EggAutonomousState{
public:
    EggAutonomousState(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        EggAutonomousStats& stats,
        EventNotificationOption& notification_nonshiny_keep,
        EventNotificationOption& notification_shiny,
        EventNotificationOption& notification_error,
        uint16_t scroll_to_read_delay,
        Language language,
        ShortcutDirection& shortcut,
        uint16_t travel_time_per_fetch,
        const EggHatchFilterOption& filters,
        uint8_t max_keepers,
        uint8_t existing_eggs_in_columns
    );

    bool column_is_filled() const { return m_eggs_in_column >= 5; }
    size_t babies_saved() const{ return m_babies_saved; }

    void dump() const;

    void set(const EggAutonomousState& state);

public:
    bool overworld_detect_and_run_state();

public:
    //  All of these must be run from the overworld with the menu closed.
    void fetch_egg();
    void hatch_egg();
    void hatch_rest_of_party();
    void spin_until_fetch_or_hatch();

    bool process_batch();   //  Returns true if program should stop.

public:
    //  Run inside the box system.
    void withdraw_egg_column();
    bool process_party();   //  Returns true if program should stop.

private:
    [[noreturn]] void process_error(const std::string& name, const char* message);
    void process_shiny(QImage screen);

private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    BotBaseContext& m_context;
    EggAutonomousStats& m_stats;
    EventNotificationOption m_notification_noop;
    EventNotificationOption& m_notification_nonshiny_keep;
    EventNotificationOption& m_notification_shiny;
    EventNotificationOption& m_notification_error;
    uint16_t m_scroll_to_read_delay;
    Language m_language;
    ShortcutDirection& m_shortcut;
    uint16_t m_travel_time_per_fetch;
    const EggHatchFilterOption& m_filters;
    uint8_t m_max_keepers;

    bool m_on_bike = true;
    uint8_t m_eggs_in_column = 0;
    uint8_t m_eggs_in_party = 5;
    uint8_t m_babies_saved = 0;
};




}
}
}
#endif
