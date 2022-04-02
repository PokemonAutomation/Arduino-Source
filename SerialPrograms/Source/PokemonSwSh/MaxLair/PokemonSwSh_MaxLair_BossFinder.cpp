/*  Max Lair (Boss Finder)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "Program/PokemonSwSh_MaxLair_Run_Adventure.h"
#include "PokemonSwSh_MaxLair_BossFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairBossFinder_Descriptor::MaxLairBossFinder_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-BossFinder",
        STRING_POKEMON + " SwSh", "Max Lair - Boss Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MaxLair-BossFinder.md",
        "Auto Max Lair 2.0 - Run adventures until you find the boss you want. Once you find your boss, switch to the other programs to shiny hunt it.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}



class MaxLairBossFinder_ConsoleOptions : public ConsoleSpecificOptions{
public:
    MaxLairBossFinder_ConsoleOptions(QString label, const LanguageSet& languages)
        : ConsoleSpecificOptions(std::move(label), languages)
        , normal_ball("<b>Normal Ball:</b> Ball for catching non-boss " + STRING_POKEMON + ".", "poke-ball")
    {
        PA_ADD_OPTION(normal_ball);
    }

    PokemonBallSelect normal_ball;
};

class MaxLairBossFinder_ConsoleFactory : public ConsoleSpecificOptionsFactory{
public:
    virtual std::unique_ptr<ConsoleSpecificOptions> make(QString label, const LanguageSet& languages) const override{
        return std::unique_ptr<ConsoleSpecificOptions>(new MaxLairBossFinder_ConsoleOptions(std::move(label), languages));
    }
};


MaxLairBossFinder::MaxLairBossFinder(const MaxLairBossFinder_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , CONSOLES(MaxLairBossFinder_ConsoleFactory())
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

    PA_ADD_OPTION(CONSOLES);
    PA_ADD_OPTION(BOSS_LIST);
    PA_ADD_OPTION(HOSTING);

    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);
    PA_ADD_OPTION(NOTIFICATIONS);
}

QString MaxLairBossFinder::check_validity() const{
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
void MaxLairBossFinder::update_active_consoles(){
    size_t consoles = system_count();
    CONSOLES.set_active_consoles(consoles);
}
std::unique_ptr<StatsTracker> MaxLairBossFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




class EndBattleDecider_BossFinder : public EndBattleDecider{
public:
    EndBattleDecider_BossFinder(const Consoles& consoles, const BossActionOption& boss_list)
        : m_consoles(consoles)
        , m_boss_list(boss_list)
    {}
    virtual const std::string& normal_ball(
        size_t console_index
    ) const override{
        return console(console_index).normal_ball.slug();
    }
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const override{
        return get_filter(boss_slug).ball;
    }
    virtual CaughtScreenAction end_adventure_action(
        size_t console_index, const std::string& boss_slug,
        const PathStats& path_stats,
        bool any_shiny, bool boss_is_shiny
    ) const override{
        if (boss_slug.empty()){
            return CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
        }
        switch (get_filter(boss_slug).action){
        case BossAction::CATCH_AND_STOP_PROGRAM:
            return CaughtScreenAction::STOP_PROGRAM;
        case BossAction::CATCH_AND_STOP_IF_SHINY:
            return boss_is_shiny
                ? CaughtScreenAction::STOP_PROGRAM
                : CaughtScreenAction::TAKE_NON_BOSS_SHINY_AND_CONTINUE;
        }
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid enum.");
    }


private:
    const BossFilter& get_filter(const std::string& boss_slug) const{
        for (const BossFilter& filter : m_boss_list.list()){
            if (boss_slug == filter.slug){
                return filter;
            }
        }
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unrecognized boss slug: " + boss_slug);
    }

private:
    const MaxLairBossFinder_ConsoleOptions& console(size_t index) const{
        return static_cast<const MaxLairBossFinder_ConsoleOptions&>(m_consoles[index]);
    }

    const Consoles& m_consoles;
    const BossActionOption& m_boss_list;

};





void MaxLairBossFinder::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
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

    EndBattleDecider_BossFinder decider(CONSOLES, BOSS_LIST);

    loop_adventures(
        env, CONSOLES,
        CONSOLES.HOST, 0,
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
