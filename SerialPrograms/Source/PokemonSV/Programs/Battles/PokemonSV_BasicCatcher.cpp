/*  Basic Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV_BasicCatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Most of this is copy-pasted from Sword/Shield.


//  Returns the # of slots scrolled. Returns -1 if not found.
int move_to_ball(
    const BattleBallReader& reader,
    VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug,
    bool forward, int attempts, uint16_t delay
){
    VideoSnapshot frame = stream.video().snapshot();
    std::string first_ball = reader.read_ball(frame);
    if (first_ball == ball_slug){
        return 0;
    }

    size_t repeat_counter = 0;
    for (int c = 1; c < attempts; c++){
        pbf_press_dpad(context, forward ? DPAD_RIGHT : DPAD_LEFT, 10, delay);
        context.wait_for_all_requests();
        frame = stream.video().snapshot();
        std::string current_ball = reader.read_ball(frame);
        if (current_ball == ball_slug){
            return c;
        }
        if (current_ball == first_ball){
            repeat_counter++;
            if (repeat_counter == 10){
                return -1;
            }
        }
    }
    return -1;
}

//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader, VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug
){
    //  Search forward at high speed.
    int ret = move_to_ball(reader, stream, context, ball_slug, true, 100, 40);
    if (ret < 0){
        return 0;
    }
    if (ret == 0){
        uint16_t quantity = reader.read_quantity(stream.video().snapshot());
        return quantity == 0 ? -1 : quantity;
    }

    //  Wait a second to let the video catch up.
    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    //  Now try again in reverse at a lower speed in case we overshot.
    //  This will return immediately if we got it right the first time.
    ret = move_to_ball(reader, stream, context, ball_slug, false, 5, TICKS_PER_SECOND);
    if (ret < 0){
        return 0;
    }
    if (ret > 0){
        stream.log("Fast ball scrolling overshot by " + std::to_string(ret) + " slot(s).", COLOR_RED);
    }
    uint16_t quantity = reader.read_quantity(stream.video().snapshot());
    return quantity == 0 ? -1 : quantity;
}


//  Throw a ball. If error or not found, throw an exception.
//  Returns the quantity prior to throwing the ball.
int16_t throw_ball(
    VideoStream& stream, ProControllerContext& context,
    Language language, const std::string& ball_slug
){
    BattleBallReader reader(stream, language, COLOR_RED);
    size_t attempts = 0;
    while (true){
        context.wait_for_all_requests();

        NormalBattleMenuWatcher normal_battle_menu(COLOR_GREEN);
        MoveSelectWatcher move_select_menu(COLOR_BLUE);
        TeraCatchWatcher tera_catch_detector(COLOR_GREEN);
        int16_t quantity = 0;
        int ret = run_until<ProControllerContext>(
            stream, context, [&](ProControllerContext& context){
                quantity = move_to_ball(reader, stream, context, ball_slug);
            },
            {normal_battle_menu, move_select_menu, tera_catch_detector}
        );
        switch (ret){
        case 0:
            stream.log("Detected battle menu. Opening up ball selection...");
            pbf_press_button(context, BUTTON_X, 20, 30);
            break;
        case 1:
            stream.log("Detected move select. Backing out...");
            pbf_mash_button(context, BUTTON_B, 125);
            break;
        case 2:
            stream.log("Tera catch menu. Opening up ball selection...");
            tera_catch_detector.move_to_slot(stream, context, 0);
            pbf_press_button(context, BUTTON_A, 20, 30);
            break;
        default:
            if (quantity > 0){
                stream.log("Throwing ball...", COLOR_BLUE);
                pbf_mash_button(context, BUTTON_A, 30);
                return quantity;
            }
            if (quantity == 0){
                stream.log("Ball not found...", COLOR_RED);
                //  This is dead code for now.
                return 0;
            }
            if (attempts >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to find desired ball after multiple attempts. Did you run out?",
                    stream
                );
            }
            attempts++;
            pbf_mash_button(context, BUTTON_B, 3 * TICKS_PER_SECOND);
        }
    }
}










CatchResults basic_catcher(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit,
    bool use_first_move_if_cant_throw,
    std::function<void()> on_throw_lambda
){
    uint16_t balls_used = 0;

    bool caught = false;
    bool overworld_seen = false;
    bool last_move_attack = false;
    int last_state = -1;
    WallClock last_battle_menu = WallClock::min();
    while (true){
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        OverworldWatcher overworld(stream.logger(), COLOR_YELLOW);
        GradientArrowWatcher next_pokemon(COLOR_GREEN, GradientArrowType::RIGHT, {0.50, 0.51, 0.30, 0.10});
        GradientArrowWatcher add_to_party(COLOR_BLUE, GradientArrowType::RIGHT, {0.50, 0.39, 0.30, 0.10});
        AdvanceDialogWatcher dialog(COLOR_PURPLE);
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(120),
            {
                battle_menu,
                overworld,
                next_pokemon,
                add_to_party,
                dialog,
            }
        );
        switch (ret){
        case 0:{
            stream.log("Detected Battle Menu...");
            if (caught){
                stream.log("Detected battle menu after catch. Did you get chain attacked?", COLOR_RED);
                return CatchResults{CatchResult::POKEMON_CAUGHT, balls_used};
            }
            if (overworld_seen){
                stream.log("Detected battle after overworld. Did you get chain attacked?", COLOR_RED);
                return CatchResults{CatchResult::POKEMON_FAINTED, balls_used};
            }
            if (balls_used >= ball_limit){
                stream.log("Reached the limit of " + std::to_string(ball_limit) + " balls.", COLOR_RED);
                return CatchResults{CatchResult::BALL_LIMIT_REACHED, balls_used};
            }

            WallClock now = current_time();
            WallClock previous = last_battle_menu;
            last_battle_menu = now;
//            cout << "last_state = " << last_state << endl;
//            cout << "last_move_attack = " << last_move_attack << endl;
//            cout << "last_move_attack = " << std::chrono::duration_cast<std::chrono::milliseconds>(now - previous).count() << endl;
            if (last_state == 0 && !last_move_attack && now < previous + std::chrono::seconds(8)){
                if (!use_first_move_if_cant_throw){
                    stream.log("BasicCatcher: Unable to throw ball.", COLOR_RED);
                    return {CatchResult::CANNOT_THROW_BALL, balls_used};
                }

                stream.log("BasicCatcher: Unable to throw ball. Attempting to use first move instead...", COLOR_RED);
                battle_menu.move_to_slot(stream, context, 0);
                pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                pbf_mash_button(context, BUTTON_B, 3 * TICKS_PER_SECOND);
                last_move_attack = true;
                break;
            }

            last_move_attack = false;
            pbf_press_button(context, BUTTON_X, 20, 105);
#if 0
            context.wait_for_all_requests();
            BattleBallReader reader(stream, language, COLOR_RED);
            int16_t qty = move_to_ball(reader, stream, context, ball_slug);
            if (qty <= 0){
                return CatchResults{CatchResult::OUT_OF_BALLS, balls_used};
            }
            pbf_mash_button(context, BUTTON_A, 30);
#else
            int16_t qty = throw_ball(stream, context, language, ball_slug);
            if (qty <= 0){
                return CatchResults{CatchResult::OUT_OF_BALLS, balls_used};
            }
#endif
            pbf_mash_button(context, BUTTON_B, 500);
            balls_used++;
            if (on_throw_lambda){
                on_throw_lambda();
            }
            break;
        }
        case 1:
            if (!overworld_seen && !caught){
                stream.log("Detected Overworld... Waiting 5 seconds to confirm.");
                context.wait_for(std::chrono::seconds(5));
                overworld_seen = true;
                break;
            }
            stream.log("Detected Overworld...");
            if (caught){
                return CatchResults{CatchResult::POKEMON_CAUGHT, balls_used};
            }else{
                return CatchResults{CatchResult::POKEMON_FAINTED, balls_used};
            }
        case 2:
            stream.log("Detected own " + STRING_POKEMON + " fainted...");
            return CatchResults{CatchResult::OWN_FAINTED, balls_used};
        case 3:
            stream.log("Detected add to party...");
            caught = true;
            pbf_press_button(context, BUTTON_B, 20, 230);
            break;
        case 4:
            stream.log("Detected dialog...");
            caught = true;
            pbf_press_button(context, BUTTON_B, 20, 230);
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "basic_catcher(): No state detected after 2 minutes.",
                stream
            );
        }

        last_state = ret;
    }



}



























}
}
}
