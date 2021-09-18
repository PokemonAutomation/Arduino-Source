/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh_DenTools.h"
#include "PokemonSwSh_DenRoller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


DenRoller_Descriptor::DenRoller_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:DenRoller",
        "Den Roller",
        "SwSh-Arduino/wiki/Basic:-DenRoller",
        "Roll den to the N'th day, SR and repeat.",
        FeedbackType::OPTIONAL_,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



DenRoller::DenRoller(const DenRoller_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SKIPS(
        "<b>Number of Skips:</b>",
        3, 0, 60
    )
    , FILTER(
        "<b>Desired " + STRING_POKEMON + ":</b><br>"
        "Automatically stop when this " + STRING_POKEMON + " is rolled. Video output is required."
    )
    , VIEW_TIME(
        "<b>View Time:</b><br>Wait this long before restting. This wait is skipped if the desired " +
        STRING_POKEMON + " is set since the program will be watching it for you.",
        "5 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , READ_DELAY(
        "<b>Read Delay:</b><br>Wait this long before attempting to " +
        STRING_POKEMON + ". This needs to be long enough for the silhouette to load.",
        "1 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(SKIPS);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(CATCHABILITY);
    PA_ADD_OPTION(VIEW_TIME);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(READ_DELAY);
}



struct DenRoller::Stats : public StatsTracker{
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
    uint64_t& rolls;
    uint64_t& skips;
    uint64_t& errors;
    uint64_t& matches;
};
std::unique_ptr<StatsTracker> DenRoller::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void DenRoller::ring_bell(const BotBaseContext& context, int count) const{
    for (int c = 0; c < count; c++){
        pbf_press_button(context, BUTTON_LCLICK, 5, 10);
    }
    pbf_wait(context, 200);
}

void DenRoller::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    std::string desired_slug = FILTER.slug();

    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
    }

    rollback_date_from_home(env.console, SKIPS);
    resume_game_front_of_den_nowatts(env.console, TOLERATE_SYSTEM_UPDATE_MENU_SLOW);


    while (true){
        roll_den(env.console, 0, 0, SKIPS, CATCHABILITY);

        if (FILTER == 0){
            ring_bell(env.console, 20);
        }else{
            env.console.botbase().wait_for_all_requests();
        }
        stats.rolls++;
        stats.skips += SKIPS;

        {
            DenMonReader reader(env.console, env.console);

            enter_den(env.console, 0, SKIPS != 0, false);

            if (FILTER != 0){
                pbf_wait(env.console, READ_DELAY);
            }
            env.console.botbase().wait_for_all_requests();

            QImage screen = env.console.video().snapshot();
            DenMonReadResults results = reader.read(screen);

            //  Give user time to look at the mon.
            if (FILTER == 0){
                //  No filter enabled. Keep going.
                pbf_wait(env.console, VIEW_TIME);
            }else if (results.slugs.slugs.empty() || results.slugs.slugs.begin()->first > 50){
                //  No detection. Keep going.
                stats.errors++;
                dump_image(env.console, screen, "ReadDenMon");
                pbf_wait(env.console, VIEW_TIME);
            }else{
                //  Check if we got what we wanted.
                for (const auto& item : results.slugs.slugs){
                    if (item.second == desired_slug){
                        stats.matches++;
                        goto StopProgram;
                    }
                }
            }
        }
        env.update_stats();

        //  Add a little extra wait time since correctness matters here.
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE - 10);

        rollback_date_from_home(env.console, SKIPS);
//        reset_game_from_home(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_SLOW
        );
    }

StopProgram:
    env.update_stats();
    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
