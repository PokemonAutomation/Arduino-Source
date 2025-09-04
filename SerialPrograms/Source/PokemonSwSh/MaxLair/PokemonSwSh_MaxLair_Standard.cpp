/*  Max Lair (Normal Mode)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "Program/PokemonSwSh_MaxLair_Run_Adventure.h"
#include "PokemonSwSh_MaxLair_Standard.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairStandard_Descriptor::MaxLairStandard_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-Standard",
        STRING_POKEMON + " SwSh", "Max Lair - Standard",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MaxLair-Standard.md",
        "Auto Max Lair 2.0 - Run Dynamax Adventures until a shiny legendary is found.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        1, 4, 1
    )
{}
std::unique_ptr<StatsTracker> MaxLairStandard_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




class MaxLairStandard_ConsoleOptions : public ConsoleSpecificOptions{
public:
    MaxLairStandard_ConsoleOptions(
        std::string label,
        const LanguageSet& languages,
        bool host
    )
        : ConsoleSpecificOptions(std::move(label), languages, host)
        , normal_ball(
            "<b>Normal Ball:</b> Ball for catching non-boss " + STRING_POKEMON + ".",
            LockMode::LOCK_WHILE_RUNNING,
            "poke-ball"
        )
        , boss_ball(
            "<b>Boss Ball:</b> Ball for catching the boss/legendary " + STRING_POKEMON + ".",
            LockMode::LOCK_WHILE_RUNNING,
            "poke-ball"
        )
        , actions(true, false)
    {
        PA_ADD_OPTION(normal_ball);
        PA_ADD_OPTION(boss_ball);
        PA_ADD_OPTION(actions);
    }

    PokemonBallSelectOption normal_ball;
    PokemonBallSelectOption boss_ball;
    CaughtScreenActionsOption actions;
};
class MaxLairStandard_ConsoleFactory : public ConsoleSpecificOptionsFactory{
public:
    virtual std::unique_ptr<ConsoleSpecificOptions> make(
        std::string label,
        const LanguageSet& languages,
        bool is_host
    ) const override{
        return std::unique_ptr<ConsoleSpecificOptions>(new MaxLairStandard_ConsoleOptions(std::move(label), languages, is_host));
    }
};


MaxLairStandard::MaxLairStandard()
    : MultiSwitchProgramInstance({"Notifs", "LiveHost"})
    , GO_HOME_WHEN_DONE(false)
    , CONSOLES(MaxLairStandard_ConsoleFactory())
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
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(BOSS_SLOT);

    PA_ADD_OPTION(CONSOLES);
    PA_ADD_OPTION(HOSTING);

    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);
    PA_ADD_OPTION(NOTIFICATIONS);
}

std::string MaxLairStandard::check_validity() const{
    std::string error = MultiSwitchProgramInstance::check_validity();
    if (!error.empty()){
        return error;
    }

    size_t active_consoles = CONSOLES.active_consoles();
    error = CONSOLES.HOST.check_validity(active_consoles);
    if (!error.empty()){
        return error;
    }
    error = HOSTING.check_validity(active_consoles);
    if (!error.empty()){
        return error;
    }
    return std::string();
}
void MaxLairStandard::update_active_consoles(size_t switch_count){
    CONSOLES.set_active_consoles(switch_count);
}





class EndBattleDecider_Standard : public EndBattleDecider{
public:
    EndBattleDecider_Standard(const Consoles& consoles)
        : m_consoles(consoles)
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
        const CaughtScreenActionsOption& actions = console(console_index).actions;
        if (boss_is_shiny){
            return actions.shiny_boss;
        }
        if (any_shiny){
            return actions.shiny_nonboss;
        }
        return actions.no_shinies;
    }

private:
    const MaxLairStandard_ConsoleOptions& console(size_t index) const{
        return static_cast<const MaxLairStandard_ConsoleOptions&>(m_consoles[index]);
    }

    const Consoles& m_consoles;
};




void MaxLairStandard::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    size_t host_index = CONSOLES.HOST.current_value();
    if (host_index >= env.consoles.size()){
        throw UserSetupError(env.logger(), "Invalid Host Switch");
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        assert_16_9_720p_min(console, console);
        if (START_LOCATION.start_in_grip_menu()){
            grip_menu_connect_go_home(context);
            resume_game_no_interact(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }else{
            pbf_press_button(context, BUTTON_B, 5, 5);
        }
    });

    EndBattleDecider_Standard decider(CONSOLES);

    loop_adventures(
        env, scope, CONSOLES,
        host_index, BOSS_SLOT.current_value(),
        decider,
        GO_HOME_WHEN_DONE,
        HOSTING,
        TOUCH_DATE_INTERVAL,
        NOTIFICATION_STATUS,
        NOTIFICATION_SHINY
    );

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
