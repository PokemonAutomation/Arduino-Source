/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_DenRoller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


DenRoller_Descriptor::DenRoller_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:DenRoller",
        STRING_POKEMON + " SwSh", "Den Roller",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DenRoller.md",
        "Roll den to the N'th day, SR and repeat.",
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct DenRoller_Descriptor::Stats : public StatsTracker{
    Stats()
        : rolls(m_stats["Rolls"])
        , skips(m_stats["Day Skips"])
        , errors(m_stats["Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Rolls"));
        m_display_order.emplace_back(Stat("Day Skips"));
        m_display_order.emplace_back(Stat("Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }
    std::atomic<uint64_t>& rolls;
    std::atomic<uint64_t>& skips;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> DenRoller_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



DenRoller::DenRoller()
    : SKIPS(
        "<b>Number of Skips:</b>",
        LockWhileRunning::LOCKED,
        3, 0, 60
    )
    , FILTER(
        "<b>Desired " + STRING_POKEMON + ":</b><br>"
        "Automatically stop when this " + STRING_POKEMON + " is rolled. Video output is required."
    )
    , VIEW_TIME(
        "<b>View Time:</b><br>Wait this long before restting. This wait is skipped if the desired " +
        STRING_POKEMON + " is set since the program will be watching it for you.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , READ_DELAY(
        "<b>Read Delay:</b><br>Wait this long before attempting to " +
        STRING_POKEMON + ". This needs to be long enough for the silhouette to load.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "1 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(CATCHABILITY);
    PA_ADD_OPTION(VIEW_TIME);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(READ_DELAY);
}




void DenRoller::ring_bell(BotBaseContext& context, int count) const{
    for (int c = 0; c < count; c++){
        pbf_press_button(context, BUTTON_LCLICK, 5, 10);
    }
    pbf_wait(context, 200);
}

void DenRoller::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    DenRoller_Descriptor::Stats& stats = env.current_stats<DenRoller_Descriptor::Stats>();

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_FAST);
    }

    rollback_date_from_home(context, SKIPS);
    if (env.console.video().snapshot()){
        NintendoSwitch::resume_game_from_home(env.console, context);
    }else{
        resume_game_front_of_den_nowatts(context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    }


    VideoSnapshot screen;
    while (true){
        roll_den(env.console, context, 0, 0, SKIPS, CATCHABILITY);

        size_t desired_index = FILTER.index();
        std::string desired_slug = FILTER.slug();

        if (desired_index == 0){
            ring_bell(context, 20);
        }else{
            context.wait_for_all_requests();
        }
        stats.rolls++;
        stats.skips += SKIPS;

        {
            DenMonReader reader(env.console, env.console);

            enter_den(context, 0, SKIPS != 0, false);

            if (desired_index != 0){
                pbf_wait(context, READ_DELAY);
            }
            context.wait_for_all_requests();

            screen = env.console.video().snapshot();
            DenMonReadResults results = reader.read(screen);

            //  Give user time to look at the mon.
            if (desired_index == 0){
                //  No filter enabled. Keep going.
                pbf_wait(context, VIEW_TIME);
            }else if (results.slugs.results.empty()){
                //  No detection. Keep going.
                stats.errors++;
                dump_image(env.console, env.program_info(), "ReadDenMon", screen);
                pbf_wait(context, VIEW_TIME);
            }else{
                //  Check if we got what we wanted.
                for (const auto& item : results.slugs.results){
                    if (item.second == desired_slug){
                        stats.matches++;
                        goto StopProgram;
                    }
                }
            }
        }
        env.update_stats();

        //  Add a little extra wait time since correctness matters here.
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE - 10);

        rollback_date_from_home(context, SKIPS);
//        reset_game_from_home(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
        reset_game_from_home_with_inference(
            env.console, context,
            ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }

StopProgram:
    env.update_stats();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Found a match!",
        screen, false
    );
}



}
}
}
