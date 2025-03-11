/*  Egg Autonomous State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggAutonomousState_H
#define PokemonAutomation_PokemonBDSP_EggAutonomousState_H

//#include "Common/Compiler.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
    class ImageRGB32;
    class ProgramEnvironment;
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
        ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
        EggAutonomousStats& stats,
        EventNotificationOption& notification_nonshiny_keep,
        EventNotificationOption& notification_shiny,
        Milliseconds scroll_to_read_delay,
        Language language,
        ShortcutDirectionOption& shortcut,
        Milliseconds travel_time_per_fetch,
        const Pokemon::StatsHuntIvJudgeFilterTable& filters,
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
    void process_shiny(const ImageViewRGB32& screen);

private:
    ProgramEnvironment& m_env;
    VideoStream& m_stream;
    ProControllerContext& m_context;
    EggAutonomousStats& m_stats;
    static EventNotificationOption m_notification_noop;
    EventNotificationOption& m_notification_nonshiny_keep;
    EventNotificationOption& m_notification_shiny;
    Milliseconds m_scroll_to_read_delay;
    Language m_language;
    ShortcutDirectionOption& m_shortcut;
    Milliseconds m_travel_time_per_fetch;
    const Pokemon::StatsHuntIvJudgeFilterTable& m_filters;
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
