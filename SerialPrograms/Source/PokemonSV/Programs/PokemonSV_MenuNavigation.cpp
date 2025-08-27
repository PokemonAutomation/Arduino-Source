/*  PokemonSV World Navigation
 *
 */

#include "Common/Cpp/RecursiveThrottler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
//#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
//#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_NeutralDateSkip.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/PokemonSV_BagDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV_ConnectToInternet.h"
#include "PokemonSV_MenuNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


void set_time_to_12am_from_home(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context){
    DateReader reader (console);
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);

//    pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(console, context, true);
    pbf_press_button(context, BUTTON_A, 20, 50);

    context.wait_for_all_requests();

    DateTime time = reader.read_date(console, console.video().snapshot()).second;
    time.hour = 0;
    reader.set_date(info, console, context, time);
//    reader.set_hours(info, console, context, 0);

    pbf_press_button(context, BUTTON_A, 20, 30);
    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
//    resume_game_from_home(console, context);
}

void neutral_day_skip_switch1(ConsoleHandle& console, ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("PokemonSV::neutral_day_skip_switch1()");
    }

    Milliseconds tv = context->timing_variation();
    if (tv == 0ms){
        ssf_press_button(context, BUTTON_A, 160ms, 80ms);

        //  Left scroll in case we missed the date menu and landed in the
        //  language change.
        ssf_issue_scroll(context, DPAD_LEFT, 0ms, 48ms, 24ms);

        ssf_press_button(context, BUTTON_A, 24ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 24ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 24ms);
        ssf_press_button(context, BUTTON_A, 0ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 24ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 24ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 24ms);
        ssf_issue_scroll(context, DPAD_RIGHT, 0ms, 48ms, 24ms);
        ssf_press_button(context, BUTTON_A, 160ms, 80ms);
    }else{
        ssf_press_button_ptv(context, BUTTON_A, 160ms);

        //  Left scroll in case we missed the date menu and landed in the
        //  language change.
        ssf_issue_scroll_ptv(context, DPAD_LEFT);

        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_press_button_ptv(context, BUTTON_A);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_press_button_ptv(context, BUTTON_A);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_press_button_ptv(context, BUTTON_A, 160ms);
    }
}
void neutral_day_skip_switch2(ConsoleHandle& console, ProControllerContext& context){
    ThrottleScope scope(context->logging_throttler());
    if (scope){
        context->logger().log("PokemonSV::neutral_day_skip_switch2()");
    }

    ssf_press_button(context, BUTTON_A, 216ms, 80ms);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
    ssf_press_button(context, BUTTON_A, 264ms, 80ms);
}

void day_skip_from_overworld(ConsoleHandle& console, ProControllerContext& context){
    go_home(console, context);
    home_to_date_time(console, context, true);

    ConsoleType console_type = console.state().console_type();
    if (is_switch1(console_type)){
        neutral_day_skip_switch1(console, context);
    }else if (is_switch2(console_type)){
        neutral_day_skip_switch2(console, context);

#if 0
        for (int c = 0; c < 10; c++){
            NintendoSwitch::roll_date_forward_1(console, context, true);
//            NintendoSwitch::neutral_date_skip(console, context);
        }
#endif
    }else{
        throw UserSetupError(
            console,
            "Please select a valid Switch console type."
        );
    }

    pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    resume_game_from_home(console, context);
}

void press_Bs_to_back_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context, uint16_t seconds_between_b_presses){
    context.wait_for_all_requests();
    OverworldWatcher overworld(stream.logger(), COLOR_RED);
    NormalBattleMenuWatcher battle(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [seconds_between_b_presses](ProControllerContext& context){
            pbf_wait(context, seconds_between_b_presses * TICKS_PER_SECOND); // avoiding pressing B if already in overworld
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_B, 20, seconds_between_b_presses * TICKS_PER_SECOND);
            }
        },
        {overworld, battle}
    );
    if (ret == 1){
        throw_and_log<UnexpectedBattleException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "press_Bs_to_back_to_overworld(): Unexpectedly detected battle.",
            stream
        );         
    }else if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "press_Bs_to_back_to_overworld(): Unable to detect overworld after 10 button B presses.",
            stream
        );
    }
}

void open_map_from_overworld(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    bool clear_tutorial
){
    {
        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        NormalBattleMenuWatcher battle(COLOR_RED);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(10),
            {overworld, battle}
        );
        context.wait_for(std::chrono::milliseconds(100));

        if (ret == 0){
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_Y, 20, 105); // open map
        }else if (ret == 1){
            throw_and_log<UnexpectedBattleException>(
                stream.logger(), ErrorReport::NO_ERROR_REPORT,
                "open_map_from_overworld(): Unexpectedly detected battle.",
                stream
            );              
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_map_from_overworld(): No overworld state found after 10 seconds.",
                stream
            );
        }
    }

    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::minutes(2)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_map_from_overworld(): Failed to open map after 2 minutes.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        AdvanceDialogWatcher advance_dialog(COLOR_YELLOW);
        PromptDialogWatcher prompt_dialog(COLOR_GREEN);
        MapWatcher map(COLOR_RED);
        NormalBattleMenuWatcher battle(COLOR_RED);

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {overworld, advance_dialog, prompt_dialog, map, battle}
        );
        context.wait_for(std::chrono::milliseconds(100));

        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_Y, 20, 105); // open map
            continue;
        case 1:
            stream.log("Detected dialog. Did you fall down?", COLOR_RED);
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 2:
            stream.log("Detected dialog. Did you fall down?", COLOR_RED);
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 3:
            stream.log("Detected map.");
            stream.overlay().add_log("Map opened", COLOR_WHITE);
            if (map.map_in_fixed_view()){
                return;
            }else{ // click R joystick to change to fixed view
                if (clear_tutorial){
                    pbf_press_button(context, BUTTON_A, 20, 105);
                }
                stream.log("Map in rotate view, fix it");
                stream.overlay().add_log("Change map to fixed view", COLOR_WHITE);
                pbf_press_button(context, BUTTON_RCLICK, 20, 105);
                continue;
            }
        case 4:
            stream.log("Detected battle.");
            throw_and_log<UnexpectedBattleException>(
                stream.logger(), ErrorReport::NO_ERROR_REPORT,
                "open_map_from_overworld(): Unexpectedly detected battle.",
                stream
            ); 
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_map_from_overworld(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void enter_box_system_from_overworld(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context
){
    context.wait_for_all_requests();
    stream.log("Enter box system from overworld...");
    WallClock start = current_time();
    bool success = false;
    while (true){
        if (current_time() - start > std::chrono::minutes(3)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_box_system_from_overworld(): Failed to enter box system after 3 minutes.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        GradientArrowWatcher box_slot_one(COLOR_BLUE, GradientArrowType::DOWN, {0.24, 0.16, 0.05, 0.09});
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {overworld, main_menu, box_slot_one}
        );
        context.wait_for(std::chrono::milliseconds(100));
        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105); // open menu
            continue;
        case 1:
            stream.log("Detected main menu.");
            stream.overlay().add_log("Enter box", COLOR_WHITE);
            success = main_menu.move_cursor(info, stream, context, MenuSide::RIGHT, 1, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "enter_box_system_from_overworld(): Cannot move menu cursor to Boxes.",
                    stream
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 50);
            continue;
        case 2:
            stream.log("Detected box.");
            context.wait_for(std::chrono::milliseconds(200));
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_box_system_from_overworld(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void leave_box_system_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    stream.log("Leave box system to overworld...");
    press_Bs_to_back_to_overworld(info, stream, context);
}


void open_pokedex_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Opening Pokédex...");
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(30)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_pokedex_from_overworld(): Failed to open Pokédex after 30 seconds.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        WhiteButtonWatcher pokedex(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});

        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context,
            std::chrono::seconds(30),
            {overworld, pokedex}
        );
        context.wait_for(std::chrono::milliseconds(100));
        switch (ret){
        case 0:
            // Try opening the Pokédex if overworld is detected
            pbf_press_button(context, BUTTON_MINUS, 20, 100);
            continue;
        case 1:
            stream.log("Detected Pokédex.");
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "open_pokedex_from_overworld(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void open_recently_battled_from_pokedex(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Opening recently battled...");
    LetsGoKillWatcher menu(stream.logger(), COLOR_RED, true, {0.23, 0.23, 0.04, 0.20});
    context.wait_for_all_requests();

    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            for (size_t i = 0; i < 10; i++){
                pbf_press_dpad(context, DPAD_DOWN, 20, 105);
            }
        },
        {menu}
    );
    if (ret == 0){
        stream.log("Detected Recently Battled menu icon.");
        pbf_mash_button(context, BUTTON_A, 150);
        pbf_wait(context, 200);
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "open_recently_battled_from_pokedex(): Unknown state after 10 dpad down presses.",
            stream
        );
    }
}


void leave_phone_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Exiting to overworld from Rotom Phone...");
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    NormalBattleMenuWatcher battle(COLOR_BLUE);
    GradientArrowWatcher arrow(COLOR_RED, GradientArrowType::DOWN, {0.475, 0.465, 0.05, 0.085});
    context.wait_for_all_requests();

    int ret = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            for (size_t i = 0; i < 10; i++){
                pbf_press_button(context, BUTTON_Y, 20, 1000);
            }
        },
        {overworld, battle, arrow}
    );
    switch (ret){
    case 0:
        return;
    case 1:
        throw_and_log<UnexpectedBattleException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "leave_phone_to_overworld(): Unexpectedly detected battle.",
            stream
        );  
    case 2:
        stream.log("Stuck in battle status screen.");
        pbf_mash_button(context, BUTTON_B, 200);
        throw_and_log<UnexpectedBattleException>(
            stream.logger(), ErrorReport::SEND_ERROR_REPORT,
            "leave_phone_to_overworld(): Unexpectedly detected battle.",
            stream
        ); 
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "leave_phone_to_overworld(): Unknown state after 10 button Y presses.",
            stream
        );
    }
}


void mash_button_till_overworld(
    VideoStream& stream,
    ProControllerContext& context,
    Button button, uint16_t seconds_run
){
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    context.wait_for_all_requests();

    int ret = run_until<ProControllerContext>(
        stream, context,
        [button, seconds_run](ProControllerContext& context){
            ssf_mash1_button(context, button, seconds_run * TICKS_PER_SECOND);
            pbf_wait(context, seconds_run * TICKS_PER_SECOND);
        },
        {overworld}
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "mash_button_till_overworld(): Timed out, no recognized state found.",
            stream
        );
    }
}

void enter_menu_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side,
    bool has_minimap
){
    if (!has_minimap){
        pbf_press_button(context, BUTTON_X, 20, 105);
    }

    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::minutes(1)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_overworld(): Failed to enter specified menu after 1 minute.",
                stream
            );
        }

        OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
        MainMenuWatcher main_menu(COLOR_RED);
        NormalBattleMenuWatcher battle(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            stream, context,
            [has_minimap](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_wait(context, 3 * TICKS_PER_SECOND);
                    if (!has_minimap){ 
                        // if no minimap, can't detect overworld, so repeatedly press X to cover for button drops
                        pbf_press_button(context, BUTTON_X, 20, 100);
                    }
                }
            },
            {overworld, main_menu, battle}
        );
        context.wait_for(std::chrono::milliseconds(100));

        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected overworld.");
            pbf_press_button(context, BUTTON_X, 20, 105);
            continue;
        case 1:
            stream.log("Detected main menu.");
            if (menu_index == -1){
                return;
            }
            success = main_menu.move_cursor(info, stream, context, side, menu_index, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "enter_menu_from_overworld(): Cannot move menu cursor to specified menu.",
                    stream
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;
        case 2:
            throw_and_log<UnexpectedBattleException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_overworld(): Unexpectedly detected battle.",
                stream
            );            
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_overworld(): No recognized state after 30 seconds. Can't find overworld or main menu.",
                stream
            );
        }
    }
}

void enter_menu_from_box_system(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side
){
    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::seconds(20)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_box_system(): Failed to enter specified menu after 20 seconds.",
                stream
            );
        }

        MainMenuWatcher main_menu(COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                // repeatedly pressing B and waiting for three seconds
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 3s);
                }
            },
            {main_menu}
        );

        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected main menu when going from box system to menu");
            if (menu_index == -1){
                return;
            }
            success = main_menu.move_cursor(info, stream, context, side, menu_index, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "enter_menu_from_box_system(): Cannot move menu cursor to specified menu.",
                    stream
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;     
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_box_system(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void enter_menu_from_bag(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side
){
    WallClock start = current_time();
    bool success = false;

    while (true){
        if (current_time() - start > std::chrono::seconds(20)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_bag(): Failed to enter specified menu after 20 seconds.",
                stream
            );
        }

        MainMenuWatcher main_menu(COLOR_RED);
        BagWatcher bag_watcher(BagWatcher::FinderType::PRESENT, COLOR_RED);
        context.wait_for_all_requests();

        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                // repeatedly pressing B and waiting for three seconds
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 3s);
                }
            },
            {main_menu, bag_watcher}
        );

        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected main menu when going from box system to menu");
            if (menu_index == -1){
                return;
            }
            success = main_menu.move_cursor(info, stream, context, side, menu_index, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "enter_menu_from_bag(): Cannot move menu cursor to specified menu.",
                    stream
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            return;
        case 1:
            stream.log("still on bag.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_menu_from_bag(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void enter_bag_from_menu(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    WallClock start = current_time();
    bool success = false;
    stream.log("Entering bag from menu");

    while (true){
        if (current_time() - start > std::chrono::seconds(20)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_bag_from_menu(): Failed to enter specified menu after 20 seconds.",
                stream
            );
        }

        MainMenuWatcher main_menu(COLOR_RED);
        BagWatcher bag_watcher(BagWatcher::FinderType::PRESENT, COLOR_RED);
        context.wait_for_all_requests();

        int ret = wait_until(
            stream, context,
            Seconds(5),
            {main_menu, bag_watcher}
        );

        const bool fast_mode = false;
        switch (ret){
        case 0:
            stream.log("Detected main menu.");
            success = main_menu.move_cursor(info, stream, context, MenuSide::RIGHT, 0, fast_mode);
            if (success == false){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "enter_bag_from_menu(): Cannot move menu cursor to specified menu.",
                    stream
                );
            }
            pbf_press_button(context, BUTTON_A, 20, 105);
            continue;
        case 1:
            stream.overlay().add_log("Enter bag");
            return;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "enter_bag_from_menu(): No recognized state after 30 seconds.",
                stream
            );
        }
    }
}


void press_button_until_gradient_arrow(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    ImageFloatBox box_area_to_check,
    Button button,
    GradientArrowType arrow_type
){
    GradientArrowWatcher arrow(COLOR_RED, arrow_type, box_area_to_check);
    int ret = run_until<ProControllerContext>(
        stream, context,
        [button](ProControllerContext& context){
            pbf_wait(context, 3 * TICKS_PER_SECOND); // avoid pressing button if arrow already detected
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, button, 20, 3 * TICKS_PER_SECOND);
            }
        },
        {arrow}
    );
    if (ret == 0){
        stream.log("Gradient arrow detected.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to detect gradient arrow.",
            stream
        );
    }     
}

void navigate_school_layout_menu(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    ImageFloatBox arrow_box_start,
    ImageFloatBox arrow_box_end,
    DpadPosition dpad_button,
    uint16_t num_button_presses
){
    GradientArrowWatcher arrow_start(COLOR_RED, GradientArrowType::RIGHT, arrow_box_start);

    int ret = wait_until(stream, context, Milliseconds(5000), { arrow_start });
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "navigate_school_layout_menu: Failed to detect gradient arrow at expected start position.",
            stream
        );        
    }

    for (uint16_t i = 0; i < num_button_presses; i++){
        pbf_press_dpad(context, dpad_button, 20, 105);
    }

    GradientArrowWatcher arrow_end(COLOR_RED, GradientArrowType::RIGHT, arrow_box_end);
    ret = run_until<ProControllerContext>(
        stream,
        context,
        [dpad_button](ProControllerContext& context){
            for (int i = 0; i < 3; i++){
                pbf_press_dpad(context, dpad_button, 20, 500);
            }
        },
        { arrow_end }        
    );

    if (ret == 0){
        stream.log("navigate_school_layout_menu: Desired item selected.");
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "navigate_school_layout_menu: Failed to detect gradient arrow at expected end position.",
            stream
        );        
    }
}



}
}
}
