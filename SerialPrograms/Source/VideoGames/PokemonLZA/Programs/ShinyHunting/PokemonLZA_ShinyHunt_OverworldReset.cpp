/*  Shiny Hunt - Overworld Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_ShinyHunt_OverworldReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_OverworldReset_Descriptor::ShinyHunt_OverworldReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-OverworldReset",
        STRING_POKEMON + " LZA", "Overworld Reset",
        "Programs/PokemonLZA/ShinyHunt-OverworldReset.html",
        "Shiny hunt by soft-resetting a " + STRING_POKEMON + " that spawns next to you.",
        PanelQualityStanding::OFFICIAL,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class ShinyHunt_OverworldReset_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_OverworldReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_OverworldReset::ShinyHunt_OverworldReset()
    : RESET_DELAY(
        "<b>Reset Delay:</b><br>Wait this long after the game loads and player character moves before resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , ROTATE_CAMERA(
        "<b>Rotate Camera:</b><br>Rotate camera during waiting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , MOVEMENT_GROUP(
        "Movement After Loading",
        LockMode::UNLOCK_WHILE_RUNNING
    )
    , MOVEMENT(
        "<b>Movement:</b><br>"
        "Move the player after entering the game to close gap. ",
        {
            {Movement::NONE, "none", "No Movement"},
            {Movement::RUN, "run", "Run (Hold B)"},
            {Movement::ROLL, "roll", "Roll (Press Y)"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        Movement::NONE
    )
    , RUN_DIRECTION(
        "<b>Run Direction:</b><br>Run in this direction, relative to the camera.",
        {
            {RunDirection::FORWARD, "forward", "Forward"},
            {RunDirection::BACKWARD, "backward", "Backward"},
            {RunDirection::LEFT, "left", "Left"},
            {RunDirection::RIGHT, "right", "Right"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        RunDirection::FORWARD
    )
    , RUN_DURATION(
        "<b>Run Duration:</b><br>Run for this long.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , ROLLS(
        "<b>Rolls:</b><br>Roll this many times in the direction the player is facing.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 1
    )
    , SHINY_DETECTED(
        "Shiny Detected", "",
        "5000 ms",
        ShinySoundDetectedAction::STOP_PROGRAM
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(RESET_DELAY);
    PA_ADD_OPTION(MOVEMENT_GROUP);
    MOVEMENT_GROUP.PA_ADD_OPTION(MOVEMENT);
    MOVEMENT_GROUP.PA_ADD_OPTION(RUN_DIRECTION);
    MOVEMENT_GROUP.PA_ADD_OPTION(RUN_DURATION);
    MOVEMENT_GROUP.PA_ADD_OPTION(ROLLS);
    PA_ADD_OPTION(ROTATE_CAMERA);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);

    ShinyHunt_OverworldReset::on_config_value_changed(this);
    MOVEMENT.add_listener(*this);
}
ShinyHunt_OverworldReset::~ShinyHunt_OverworldReset(){
    MOVEMENT.remove_listener(*this);
}
void ShinyHunt_OverworldReset::on_config_value_changed(void* object){
    switch (MOVEMENT){
    case Movement::NONE:
        RUN_DIRECTION.set_visibility(ConfigOptionState::HIDDEN);
        RUN_DURATION.set_visibility(ConfigOptionState::HIDDEN);
        ROLLS.set_visibility(ConfigOptionState::HIDDEN);
        break;
    case Movement::RUN:
        RUN_DIRECTION.set_visibility(ConfigOptionState::ENABLED);
        RUN_DURATION.set_visibility(ConfigOptionState::ENABLED);
        ROLLS.set_visibility(ConfigOptionState::HIDDEN);
        break;
    case Movement::ROLL:
        RUN_DIRECTION.set_visibility(ConfigOptionState::HIDDEN);
        RUN_DURATION.set_visibility(ConfigOptionState::HIDDEN);
        ROLLS.set_visibility(ConfigOptionState::ENABLED);
        break;
    }
}


void ShinyHunt_OverworldReset::move_after_loading(ProControllerContext& context){
    switch (MOVEMENT){
    case Movement::NONE:
        return;
    case Movement::RUN:{
        JoystickPosition direction;
        switch (RUN_DIRECTION){
        case RunDirection::FORWARD:
            direction = {0, +1};
            break;
        case RunDirection::BACKWARD:
            direction = {0, -1};
            break;
        case RunDirection::LEFT:
            direction = {-1, 0};
            break;
        case RunDirection::RIGHT:
            direction = {+1, 0};
            break;
        }
        //  Hold B alongside the joystick for the whole duration so the player
        //  runs instead of walks.
        Milliseconds duration = RUN_DURATION;
        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
        pbf_move_left_joystick(context, direction, duration, 0ms);
        return;
    }
    case Movement::ROLL:
        //  Each roll takes about a second to finish before the next one can start.
        for (uint8_t c = 0; c < ROLLS; c++){
            pbf_press_button(context, BUTTON_Y, 100ms, 1000ms);
        }
        return;
    }
}

void ShinyHunt_OverworldReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    require_player(env.console, context, BUTTON_L);

    ShinyHunt_OverworldReset_Descriptor::Stats& stats = env.current_stats<ShinyHunt_OverworldReset_Descriptor::Stats>();

    uint8_t shiny_count = 0;

    while (true){
        go_home(env.console, context);
        send_program_status_notification(env, NOTIFICATION_STATUS);


        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            shiny_count++;
            stats.shinies++;
            env.update_stats();
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                reset_game_from_home(env, env.console, context);

                move_after_loading(context);
                if (ROTATE_CAMERA){
                    pbf_move_right_joystick(context, {+1, 0}, RESET_DELAY, 0ms);
                }else{
                    pbf_wait(context, RESET_DELAY);
                }

                stats.resets++;
                env.update_stats();
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();

        if (ret < 0){
            continue;
        }

        if (SHINY_DETECTED.on_shiny_sound(
            env, env.console, context,
            shiny_count,
            shiny_coefficient
        )){
            break;
        }
    }

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}










}
}
}
