/*  Shiny Hunt - Custom Path
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA_ShinyHunt-CustomPath.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


ShinyHuntCustomPath_Descriptor::ShinyHuntCustomPath_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-CustomPath",
        STRING_POKEMON + " LA", "Shiny Hunt - Custom Path",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-CustomPath.md",
        "Repeatedly travel on a custom path to shiny hunt " + STRING_POKEMON + " around it.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
class ShinyHuntCustomPath_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies", true);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntCustomPath_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHuntCustomPath::ShinyHuntCustomPath(const ShinyHuntCustomPath_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , RESET_METHOD(
          "<b>Reset Method:</b>",
          {
              "Soft Reset",
              "Go back to village",
          },
          0
    )
    , TEST_PATH(
        "<b>Test Path:</b><br>Run the path immediately on the map to test it.",
        false
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while you are ignoring shinies.",
        "0 * TICKS_PER_SECOND"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if a shiny is detected while you are listening for shinies.",
        "0 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_DESTINATION.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{

    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
//    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(PATH);
    PA_ADD_STATIC(RESET_METHOD);
    PA_ADD_OPTION(TEST_PATH);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyHuntCustomPath::do_non_listen_action(ConsoleHandle& console, BotBaseContext& context, size_t action_index){
    const auto& row = PATH.get_action(action_index);
    console.log("Execute action " + PathAction_NAMES[(size_t)row.action]);
    switch(row.action){
        case PathAction::CHANGE_MOUNT:
        {
            MountState mountState = MountState::NOTHING;
            switch(row.mount){
            case PathMount::WYRDEER:
                mountState = MountState::WYRDEER_ON;
                break;
            case PathMount::URSALUNA:
                mountState = MountState::URSALUNA_ON;
                break;
            case PathMount::BASCULEGION:
                mountState = MountState::BASCULEGION_ON;
                break;
            case PathMount::SNEASLER:
                mountState = MountState::SNEASLER_ON;
                break;
            case PathMount::BRAVIARY:
                mountState = MountState::BRAVIARY_ON;
                break;
            default:
                break;
            }

            if (mountState == MountState::NOTHING){
                dismount(console, context);
            } else{
                change_mount(console, context, mountState);
            }
            break;
        }
#if 0
        case PathAction::ROTATE_CAMERA:
        {
            if (row.camera_turn_ticks > 0){
                pbf_move_right_joystick(context, 255, 128, uint16_t(row.camera_turn_ticks), 0);
            } else if (row.camera_turn_ticks < 0){
                pbf_move_right_joystick(context, 0, 128, uint16_t(-row.camera_turn_ticks), 0);
            }
            break;
        }
#endif
        case PathAction::MOVE_FORWARD:
        {
            switch(row.move_speed){
            case PathSpeed::NORMAL_SPEED:
                pbf_move_left_joystick(context, 128, 0, row.move_forward_ticks, 0);
                break;
            case PathSpeed::SLOW_SPEED:
                pbf_move_left_joystick(context, 128, 64, row.move_forward_ticks, 0);
                break;
            case PathSpeed::RUN:
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, row.move_forward_ticks);
                break;
            case PathSpeed::DASH:
                pbf_press_button(context, BUTTON_B, row.move_forward_ticks, 0);
                break;
            case PathSpeed::DASH_B_SPAM:
                pbf_mash_button(context, BUTTON_B, row.move_forward_ticks);
                break;
            case PathSpeed::DIVE:
                pbf_press_button(context, BUTTON_Y, row.move_forward_ticks, 0);
                break;
            }
            break;
        }
        case PathAction::MOVE_IN_DIRECTION:
        {
            uint8_t x = (uint8_t)((row.left_x + 1.0) * 127.5 + 0.5);
            uint8_t y = (uint8_t)((-row.left_y + 1.0) * 127.5 + 0.5);
            pbf_move_left_joystick(context, x, y, row.move_forward_ticks, 0);
            break;
        }
        case PathAction::CENTER_CAMERA:
        {
            pbf_mash_button(context, BUTTON_ZL, 200);
            break;
        }
        case PathAction::JUMP:
        {
            pbf_press_button(context, BUTTON_Y, 10, row.jump_wait_ticks);
            break;
        }
        case PathAction::WAIT:
        {
            pbf_wait(context, row.wait_ticks);
            break;
        }
        default:   
            break;
    } // end switch action
    context.wait_for_all_requests();
}


void ShinyHuntCustomPath::run_path(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ShinyHuntCustomPath_Descriptor::Stats& stats = env.current_stats<ShinyHuntCustomPath_Descriptor::Stats>();

    std::atomic<bool> listen_for_shiny(false);
    float shiny_coefficient = 1.0;
    ShinyDetectedActionOption* shiny_action = nullptr;
    ShinySoundDetector shiny_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        shiny_coefficient = error_coefficient;
        if (listen_for_shiny.load(std::memory_order_acquire)){
            shiny_action = &SHINY_DETECTED_DESTINATION;
        }else{
            shiny_action = &SHINY_DETECTED_ENROUTE;
        }
        return on_shiny_callback(env, env.console, *shiny_action, error_coefficient);
    });

    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context){
            for (size_t action_index = 0; action_index < PATH.num_actions(); action_index++){
                const auto& row = PATH.get_action(action_index);
                if (row.action == PathAction::START_LISTEN){
                    listen_for_shiny.store(true, std::memory_order_release);
                    continue;
                }else if (row.action == PathAction::END_LISTEN){
                    listen_for_shiny.store(false, std::memory_order_release);
                    continue;
                }

                do_non_listen_action(env.console, context, action_index);
                context.wait_for_all_requests();
            }
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, *shiny_action, shiny_coefficient);
    }
}


void ShinyHuntCustomPath::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ShinyHuntCustomPath_Descriptor::Stats& stats = env.current_stats<ShinyHuntCustomPath_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    if (TEST_PATH){
        // Run the test path immediately
        env.log("Testing path...");
        run_path(env, context);
        return;
    }

    //  Check whether the user has set shiny sound listen action:
    {
        bool has_listen_action = false;
        for(size_t i = 0; i < PATH.num_actions(); i++){
            if (PATH.get_action(i).action == PathAction::START_LISTEN){
                has_listen_action = true;
                break;
            }
        }
        if (has_listen_action == false){
            throw OperationFailedException(env.console, "No START LISTEN action specified.");
        }
    }

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            stats.attempts++;

            goto_camp_from_jubilife(env, env.console, context, PATH.travel_location());
            run_path(env, context);

            if(RESET_METHOD == 0){
                env.console.log("Resetting by closing the game.");
                pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
                reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            }else{
                env.console.log("Resetting by going to village.");
                goto_camp_from_overworld(env, env.console, context);
                goto_professor(env.console.logger(), context, PATH.travel_location());
                from_professor_return_to_jubilife(env, env.console, context);
            }

        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }

    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
