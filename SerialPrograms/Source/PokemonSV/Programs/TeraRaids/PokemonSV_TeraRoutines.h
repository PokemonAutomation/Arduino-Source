/*  Tera Exit Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraExitRoutines_H
#define PokemonAutomation_PokemonSV_TeraExitRoutines_H

#include <string>
#include <atomic>
#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
    class EventNotificationOption;
    class ImageViewRGB32;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{






//  While in the overworld, attempt to enter a raid in front of you.
bool open_raid(VideoStream& stream, ProControllerContext& context);

//  While viewing a raid card, close and return to the overworld.
void close_raid(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);


enum class HostingMode{
    LOCAL,
    ONLINE_CODED,
    ONLINE_EVERYONE,
};
void open_hosting_lobby(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    HostingMode mode
);


//  From overworld or main menu => Code entry for tera raid.
void enter_tera_search(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    bool connect_to_internet
);





//  Exit a Tera raid after winning without catching.
//  The entry point is with the catch/no-catch option up.
//  Upon returning, you will be in the overworld.
void exit_tera_win_without_catching(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t stop_on_sparkly_items
);


//  Exit a Tera raid after winning by catching.
//  The entry point is with the catch/no-catch option up.
//  Does not check for shininess.
void exit_tera_win_by_catching(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug,
    size_t stop_on_sparkly_items
);



enum class TeraResult{
    NO_DETECTION,
    NOT_SHINY,
    SHINY,
};

//  Exit a Tera raid after winning by catching.
//  The entry point is with the catch/no-catch option up.
//  Returns the result of the caught Pokemon.
//  Upon returning, you will be in the overworld except if it is shiny and
//  "stop_on_shiny == true', then you will be in the summary of the shiny.
TeraResult exit_tera_win_by_catching(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, size_t stop_on_sparkly_items,
    std::atomic<uint64_t>* stat_shinies
);



TeraResult run_tera_summary(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    EventNotificationOption& notification_nonshiny,
    EventNotificationOption& notification_shiny,
    bool stop_on_shiny, const ImageViewRGB32& battle_screenshot,
    std::atomic<uint64_t>* stat_shinies
);

//  Run away from tera battle.
void run_from_tera_battle(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context,
    std::atomic<uint64_t>* stat_errors
);

bool is_sparkling_raid(VideoStream& stream, ProControllerContext& context);


}
}
}
#endif
