/*  Max Lair (Boss Finder)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "Framework/PokemonSwSh_MaxLair_Stats.h"
#include "Program/PokemonSwSh_MaxLair_Run_Adventure.h"
#include "PokemonSwSh_MaxLair_BossFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairBossFinder_Descriptor::MaxLairBossFinder_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-BossFinder",
        "Max Lair: Boss Finder (WIP)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MaxLair-BossFinder.md",
        "Auto Max Lair 2.0 - Run adventures until you find the boss you want. Once you find your boss, switch to the other programs to shiny hunt it.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}



MaxLairBossFinder::MaxLairBossFinder(const MaxLairBossFinder_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , HOST_SWITCH(
        "<b>Host Switch:</b><br>This is the Switch that hosts the raid.",
        {
            "Switch 0 (Top Left)",
            "Switch 1 (Top Right)",
            "Switch 2 (Bottom Left)",
            "Switch 3 (Bottom Right)",
        },
        0
    )
    , m_languages(PokemonNameReader::instance().languages())
    , PLAYER0("Switch 0 (Top Left)", m_languages, false)
    , PLAYER1("Switch 1 (Top Right)", m_languages, false)
    , PLAYER2("Switch 2 (Bottom Left)", m_languages, false)
    , PLAYER3("Switch 3 (Bottom Right)", m_languages, false)
    , NOTIFICATION_NO_SHINY("No Shinies", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_SHINY("Shiny Catch", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
    , NOTIFICATION_PROGRAM_FINISH("Program Finished", true, true)
    , NOTIFICATIONS({
        &HOSTING.NOTIFICATIONS.NOTIFICATION,
        &NOTIFICATION_NO_SHINY,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_PROGRAM_ERROR,
    })
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(HOST_SWITCH);

    PA_ADD_OPTION(PLAYER0);
    PA_ADD_OPTION(PLAYER1);
    PA_ADD_OPTION(PLAYER2);
    PA_ADD_OPTION(PLAYER3);
    PA_ADD_OPTION(BOSS_LIST);

    PA_ADD_OPTION(HOSTING);
    PA_ADD_OPTION(NOTIFICATIONS);
}

QString MaxLairBossFinder::check_validity() const{
    QString error = RunnablePanelInstance::check_validity();
    if (!error.isEmpty()){
        return error;
    }
    if (HOST_SWITCH >= count()){
        return "Host Switch cannot be larger than " + QString::number(count() - 1) +
        " since you only have " + QString::number(count()) + " Switch(es) enabled.";
    }
    if (count() != 1 && !HOSTING.RAID_CODE.code_enabled()){
        return "You must use a code when running with multiple Switches.";
    }
    return QString();
}
std::unique_ptr<StatsTracker> MaxLairBossFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




class EndBattleDecider_BossFinder : public EndBattleDecider{
public:
    EndBattleDecider_BossFinder(
        std::string console0_normal_ball,
        std::string console1_normal_ball,
        std::string console2_normal_ball,
        std::string console3_normal_ball,
        const BossActionOption& boss_list
    )
        : m_boss_list(boss_list)
    {
        m_normal_ball[0] = std::move(console0_normal_ball);
        m_normal_ball[1] = std::move(console1_normal_ball);
        m_normal_ball[2] = std::move(console2_normal_ball);
        m_normal_ball[3] = std::move(console3_normal_ball);
    }
    virtual CatchAction catch_boss_action() const override{
        return CatchAction::CATCH;
    }
    virtual const std::string& normal_ball(
        size_t console_index
    ) const override{
        return m_normal_ball[console_index];
    }
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const override{
        return get_filter(boss_slug).ball;
    }
    virtual EndAdventureAction end_adventure_action(
        size_t console_index
    ) const override{
        return EndAdventureAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
    }
    virtual EndAdventureAction end_adventure_action(
        size_t console_index, const std::string& boss_slug, bool shiny
    ) const override{
        switch (get_filter(boss_slug).action){
        case BossAction::CATCH_AND_STOP_PROGRAM:
            return EndAdventureAction::STOP_PROGRAM;
        case BossAction::CATCH_AND_STOP_IF_SHINY:
            return shiny
                ? EndAdventureAction::STOP_PROGRAM
                : EndAdventureAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
        case BossAction::DO_NOT_CATCH_AND_CONTINUE:
            return EndAdventureAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
        }
        PA_THROW_StringException("Invalid enum.");
    }


private:
    const BossFilter& get_filter(const std::string& boss_slug) const{
        for (const BossFilter& filter : m_boss_list.list()){
            if (boss_slug == filter.slug){
                return filter;
            }
        }
        PA_THROW_StringException("Unrecognized boss slug: " + boss_slug);
    }

private:
    std::string m_normal_ball[4];
    const BossActionOption& m_boss_list;

};





void MaxLairBossFinder::program(MultiSwitchProgramEnvironment& env){
    if (HOST_SWITCH >= env.consoles.size()){
        PA_THROW_StringException("Invalid Host Switch");
    }

    env.run_in_parallel([&](ConsoleHandle& console){
        if (START_IN_GRIP_MENU){
            grip_menu_connect_go_home(console);
            resume_game_no_interact(console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }else{
            pbf_press_button(console, BUTTON_B, 5, 5);
        }
    });

    EndBattleDecider_BossFinder decider(
        PLAYER0.normal_ball.slug(),
        PLAYER1.normal_ball.slug(),
        PLAYER2.normal_ball.slug(),
        PLAYER3.normal_ball.slug(),
        BOSS_LIST
    );

    size_t failed_count = 0;
    while (true){
        AdventureResult result = run_adventure(
            env, descriptor().display_name(),
            HOST_SWITCH, 0,
            PLAYER0,
            PLAYER1,
            PLAYER2,
            PLAYER3,
            HOSTING,
            decider,
            NOTIFICATION_NO_SHINY,
            NOTIFICATION_SHINY,
            false,
            true, GO_HOME_WHEN_DONE
        );
        switch (result){
        case AdventureResult::FINISHED:
            failed_count = 0;
            continue;
        case AdventureResult::STOP_PROGRAM:
            goto StopProgram;
        case AdventureResult::ERROR:
            failed_count++;
            if (failed_count == 3){
                send_program_telemetry(
                    env.logger(), true, Qt::red, MODULE_NAME,
                    "Error",
                    {{"Message", "Adventure failed 3 times in the row."}},
                    ""
                );
                PA_THROW_StringException("Adventure failed 3 times in the row.");
            }
            env.log("Failed to start adventure. Resetting all Switches...", Qt::red);
            env.run_in_parallel([&](ConsoleHandle& console){
                QImage screen = console.video().snapshot();
//                dump_image(console, MODULE_NAME, "ResetRecovery", screen);
                pbf_press_button(console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
                reset_game_from_home_with_inference(env, console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            });
            continue;
        }
    }

StopProgram:
    Stats& stats = env.stats<Stats>();
    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        descriptor().display_name(),
        "",
        stats.to_str()
    );
}






}
}
}
