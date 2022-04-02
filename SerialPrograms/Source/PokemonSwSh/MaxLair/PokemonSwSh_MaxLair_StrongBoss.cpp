/*  Max Lair (Strong-Boss Mode)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "Program/PokemonSwSh_MaxLair_Run_Adventure.h"
#include "PokemonSwSh_MaxLair_StrongBoss.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairStrongBoss_Descriptor::MaxLairStrongBoss_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-StrongBoss",
        STRING_POKEMON + " SwSh", "Max Lair - Strong Boss",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MaxLair-StrongBoss.md",
        "Auto Max Lair 2.0 - Run Dynamax Adventures and intelligently reset to keep paths that have high win rates.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}


class MaxLairStrongBoss_ConsoleOptions : public ConsoleSpecificOptions{
public:
    MaxLairStrongBoss_ConsoleOptions(
        QString label,
        const LanguageSet& languages
    )
        : ConsoleSpecificOptions(std::move(label), languages)
        , normal_ball("<b>Normal Ball:</b> Ball for catching non-boss " + STRING_POKEMON + ".", "poke-ball")
        , boss_ball("<b>Boss Ball:</b> Ball for catching the boss/legendary " + STRING_POKEMON + ".", "poke-ball")
        , actions_non_host(false, false)
        , actions_host(
            false, true,
            CaughtScreenAction::RESET,
            CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE,
            CaughtScreenAction::STOP_PROGRAM
        )
    {
        PA_ADD_OPTION(normal_ball);
        PA_ADD_OPTION(boss_ball);
        PA_ADD_OPTION(actions_non_host);
        PA_ADD_OPTION(actions_host);
    }

    const CaughtScreenActionsOption& actions() const{
        return is_host ? actions_host : actions_non_host;
    }
    virtual void set_host(bool is_host) override{
        ConsoleSpecificOptions::set_host(is_host);
        if (is_host){
            actions_host.visibility = ConfigOptionState::ENABLED;
            actions_non_host.visibility = ConfigOptionState::HIDDEN;
        }else{
            actions_host.visibility = ConfigOptionState::HIDDEN;
            actions_non_host.visibility = ConfigOptionState::ENABLED;
        }
    }

    PokemonBallSelect normal_ball;
    PokemonBallSelect boss_ball;
    CaughtScreenActionsOption actions_non_host;
    CaughtScreenActionsOption actions_host;
};
class MaxLairStrongBoss_ConsoleFactory : public ConsoleSpecificOptionsFactory{
public:
    virtual std::unique_ptr<ConsoleSpecificOptions> make(QString label, const LanguageSet& languages) const override{
        return std::unique_ptr<ConsoleSpecificOptions>(new MaxLairStrongBoss_ConsoleOptions(std::move(label), languages));
    }
};


MaxLairStrongBoss::MaxLairStrongBoss(const MaxLairStrongBoss_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , MIN_WIN_RATE(
        "<b>Minimum Win Rate:</b><br>"
        "Keep the path if the win rate stays above this ratio. This is done by resetting the host.",
        0.75, 0, 1.0
    )
    , CONSOLES(MaxLairStrongBoss_ConsoleFactory())
    , NOTIFICATION_STATUS("Status Update", true, false)
    , NOTIFICATION_SHINY("Shiny Catch", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
    , NOTIFICATIONS({
        &HOSTING.NOTIFICATIONS.NOTIFICATION,
        &NOTIFICATION_STATUS,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(MIN_WIN_RATE);
    PA_ADD_OPTION(BOSS_SLOT);

    PA_ADD_OPTION(CONSOLES);
    PA_ADD_OPTION(HOSTING);

    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);
    PA_ADD_OPTION(NOTIFICATIONS);
}

QString MaxLairStrongBoss::check_validity() const{
    QString error = RunnablePanelInstance::check_validity();
    if (!error.isEmpty()){
        return error;
    }
    error = CONSOLES.HOST.check_validity(system_count());
    if (!error.isEmpty()){
        return error;
    }
    error = HOSTING.check_validity(system_count());
    if (!error.isEmpty()){
        return error;
    }
    return QString();
}
void MaxLairStrongBoss::update_active_consoles(){
    size_t consoles = system_count();
    CONSOLES.set_active_consoles(consoles);
}
std::unique_ptr<StatsTracker> MaxLairStrongBoss::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





class EndBattleDecider_StrongBoss : public EndBattleDecider{
public:
    EndBattleDecider_StrongBoss(
        LoggerQt& logger,
        const Consoles& consoles, size_t host_index,
        double min_win_ratio
    )
        : m_logger(logger)
        , m_consoles(consoles)
        , m_host_index(host_index)
        , m_min_win_ratio(min_win_ratio)
    {}
    virtual const std::string& normal_ball(
        size_t console_index
    ) const override{
        return console(console_index).normal_ball.slug();
    }
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const override{
        return console(console_index).boss_ball.slug();
    }
    virtual CaughtScreenAction end_adventure_action(
        size_t console_index, const std::string& boss_slug,
        const PathStats& path_stats,
        bool any_shiny, bool boss_is_shiny
    ) const override{
        const CaughtScreenActionsOption& actions = console(console_index).actions();
        CaughtScreenAction action;
        do{
            if (boss_is_shiny){
                action = actions.shiny_boss;
                break;
            }
            if (any_shiny){
                action = actions.shiny_nonboss;
                break;
            }
            action = actions.no_shinies;
        }while (false);
        if (action != CaughtScreenAction::RESET || console_index != m_host_index){
            return action;
        }
        double win_ratio = path_stats.win_ratio();
        if (win_ratio >= m_min_win_ratio){
            m_logger.log("Win Ratio = " + QString::number(win_ratio) + ": Resetting to keep path.", COLOR_BLUE);
            return CaughtScreenAction::RESET;
        }else{
            m_logger.log("Win Ratio = " + QString::number(win_ratio) + ": Continuing to get new path.", COLOR_BLUE);
            return CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
        }
    }

private:
    const MaxLairStrongBoss_ConsoleOptions& console(size_t index) const{
        return static_cast<const MaxLairStrongBoss_ConsoleOptions&>(m_consoles[index]);
    }

    LoggerQt& m_logger;
    const Consoles& m_consoles;
    size_t m_host_index;
    double m_min_win_ratio;
};



void MaxLairStrongBoss::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    if (CONSOLES.HOST >= env.consoles.size()){
        throw UserSetupError(env.logger(), "Invalid Host Switch");
    }

    env.run_in_parallel([&](ConsoleHandle& console){
        if (START_IN_GRIP_MENU){
            grip_menu_connect_go_home(console);
            resume_game_no_interact(console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }else{
            pbf_press_button(console, BUTTON_B, 5, 5);
        }
    });

    EndBattleDecider_StrongBoss decider(
        env.logger(),
        CONSOLES, CONSOLES.HOST,
        MIN_WIN_RATE
    );

    loop_adventures(
        env, CONSOLES,
        CONSOLES.HOST, BOSS_SLOT,
        decider,
        GO_HOME_WHEN_DONE,
        HOSTING,
        TOUCH_DATE_INTERVAL,
        NOTIFICATION_STATUS,
        NOTIFICATION_SHINY
    );

    Stats& stats = env.stats<Stats>();
    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}






}
}
}
