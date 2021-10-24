/*  Max Lair
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "Program/PokemonSwSh_MaxLair_Run_Adventure.h"
#include "PokemonSwSh_MaxLair_SingleRun.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace MaxLairInternal;



MaxLairSingleRun_Descriptor::MaxLairSingleRun_Descriptor()
    : MultiSwitchProgramDescriptor(
        "PokemonSwSh:MaxLair-SingleAdventure",
        "Max Lair: Single Adventure (WIP)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/MaxLair-SingleAdventure.md",
        "Auto Max Lair 2.0 - Run a single Dynamax Adventure and stop.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}


MaxLairSingleRun::MaxLairSingleRun(const MaxLairSingleRun_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
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
    , BOSS_SLOT(
        "<b>Boss Slot:</b>",
        {
            "Random",
            "Slot 1",
            "Slot 2",
            "Slot 3",
        },
        0
    )
    , m_languages(PokemonNameReader::instance().languages())
    , PLAYER0("Switch 0 (Top Left)", m_languages, false)
    , PLAYER1("Switch 1 (Top Right)", m_languages, false)
    , PLAYER2("Switch 2 (Bottom Left)", m_languages, false)
    , PLAYER3("Switch 3 (Bottom Right)", m_languages, false)
    , NOTIFICATION_NO_SHINY("No Shinies", true, false)
    , NOTIFICATION_SHINY("Shiny Catch", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
    , NOTIFICATIONS({
        &HOSTING.NOTIFICATIONS.NOTIFICATION,
        &NOTIFICATION_NO_SHINY,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_ERROR,
    })
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);

    PA_ADD_OPTION(HOST_SWITCH);
    PA_ADD_OPTION(BOSS_SLOT);

    PA_ADD_OPTION(PLAYER0);
    PA_ADD_OPTION(PLAYER1);
    PA_ADD_OPTION(PLAYER2);
    PA_ADD_OPTION(PLAYER3);

    PA_ADD_OPTION(HOSTING);
    PA_ADD_OPTION(NOTIFICATIONS);
}

QString MaxLairSingleRun::check_validity() const{
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
std::unique_ptr<StatsTracker> MaxLairSingleRun::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



class EndBattleDecider_Stop : public EndBattleDecider{
public:
    EndBattleDecider_Stop(
        std::string console0_normal_ball,
        std::string console1_normal_ball,
        std::string console2_normal_ball,
        std::string console3_normal_ball
    )
        : m_boss_ball("poke-ball")
    {
        m_normal_ball[0] = std::move(console0_normal_ball);
        m_normal_ball[1] = std::move(console1_normal_ball);
        m_normal_ball[2] = std::move(console2_normal_ball);
        m_normal_ball[3] = std::move(console3_normal_ball);
    }

    virtual CatchAction catch_boss_action() const override{
        return CatchAction::STOP_PROGRAM;
    }
    virtual const std::string& normal_ball(
        size_t console_index
    ) const override{
        return m_normal_ball[console_index];
    }
    virtual const std::string& boss_ball(
        size_t console_index, const std::string& boss_slug
    ) const override{
        return m_boss_ball;
    }
    virtual EndAdventureAction end_adventure_action(
        size_t console_index
    ) const override{
        return EndAdventureAction::STOP_PROGRAM;
    }
    virtual EndAdventureAction end_adventure_action(
        size_t console_index, const std::string& boss_slug, bool shiny
    ) const override{
        return EndAdventureAction::STOP_PROGRAM;
    }

private:
    std::string m_normal_ball[4];
    std::string m_boss_ball;
};



void MaxLairSingleRun::program(MultiSwitchProgramEnvironment& env){
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

    EndBattleDecider_Stop decider(
        PLAYER0.normal_ball.slug(),
        PLAYER1.normal_ball.slug(),
        PLAYER2.normal_ball.slug(),
        PLAYER3.normal_ball.slug()
    );

    run_adventure(
        env, descriptor().display_name(),
        HOST_SWITCH, BOSS_SLOT,
        PLAYER0,
        PLAYER1,
        PLAYER2,
        PLAYER3,
        HOSTING,
        decider,
        NOTIFICATION_NO_SHINY,
        NOTIFICATION_SHINY,
        true,
        false, false
    );

    env.update_stats();
}



}
}
}




















