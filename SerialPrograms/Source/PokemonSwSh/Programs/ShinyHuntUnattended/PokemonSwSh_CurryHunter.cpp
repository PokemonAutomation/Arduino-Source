/*  Curry Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonTools/Async/InferenceSession.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
//#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinySparkleDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_CurryHunter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


CurryHunter_Descriptor::CurryHunter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:CurryHunter",
        STRING_POKEMON + " SwSh", "Curry Hunter",
        "Programs/PokemonSwSh/CurryHunter.html",
        "Cooks curry to attract " + STRING_POKEMON + " to your camp. "
        "<font color=\"red\">(This program cannot detect shinies. You must check manually or with " + STRING_POKEMON + " HOME.)</font>",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct CurryHunter_Descriptor::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_attempts(m_stats["Attempts"])
    {
        for (auto& item : m_display_order){
            item.display_mode = HIDDEN_IF_ZERO;
        }
        m_display_order.insert(m_display_order.begin(), Stat("Attempts"));
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> CurryHunter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


CurryHunter::CurryHunter()
    : WALK_UP_DELAY0(
        "<b>Walk up Delay:</b><br>Wait this long for the " + STRING_POKEMON + " to walk up to you.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , SMALL_POKEMON(
        "<b>Walk forward for Small " + STRING_POKEMON + ":</b><br>"
        "If there are small " + STRING_POKEMON +
        ", increase this number by 240 ms. You may have to adjust the number and check what works best for your area.",
        LockMode::LOCK_WHILE_RUNNING,
        "0 ms"
    )
    , TAKE_VIDEO(
        "<b>Take Videos:</b><br>Take a video after each cooking iteration. This will spam your album with videos.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , ITERATIONS(
        "<b>Iterations:</b><br># of cooking attempts.",
        LockMode::LOCK_WHILE_RUNNING,
        999
    )
{
    PA_ADD_OPTION(WALK_UP_DELAY0);
    PA_ADD_OPTION(SMALL_POKEMON);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(ITERATIONS);
}



void CurryHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(context, BUTTON_R, 40ms, 400ms);
    }


    CurryHunter_Descriptor::Stats& stats = env.current_stats<CurryHunter_Descriptor::Stats>();
    env.update_stats();


    //  Select the cooking option.
    pbf_press_button(context, BUTTON_X, 40ms, 1000ms);
    pbf_press_dpad(context, DPAD_RIGHT, 40ms, 800ms);
    pbf_press_button(context, BUTTON_A, 40ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 40ms, 2400ms);


    for (uint32_t it = 0; it < ITERATIONS; it++){
        //  Which ingredient do you want to use?
        pbf_press_button(context, BUTTON_A, 40ms, 600ms);     //  Get rid of the pop-up menu.
        pbf_press_button(context, BUTTON_A, 40ms, 1200ms);    //  75. english text is longer.
        pbf_press_button(context, BUTTON_A, 40ms, 800ms);    //  Ingredient is now selected.

        //  Which berries do you want to use?
        pbf_press_button(context, BUTTON_A, 40ms, 600ms);     //  Get rid of the pop-up menu.
        pbf_press_button(context, BUTTON_A, 40ms, 600ms);
        pbf_press_dpad(context, DPAD_UP, 40ms, 600ms);
        pbf_press_button(context, BUTTON_A, 40ms, 1000ms);
        pbf_press_button(context, BUTTON_PLUS, 40ms, 1600ms); //  125. english text is longer.
        pbf_press_button(context, BUTTON_A, 40ms, 8000ms);   //  Berries are now selected as well.


        //  Around 17 seconds of A mashing for the first curry cooking phase.
        for (uint16_t c = 0; c < 2100; c = c + 10){
            pbf_press_button(context, BUTTON_A, 40ms, 40ms);
        }

        //  Slowing down to not burn the curry.
        for (uint16_t c = 0; c < 300; c = c + 25){
            pbf_press_button(context, BUTTON_A, 40ms, 160ms);
        }
        pbf_wait(context, 1360ms);


        //  Do circles with the joystick. Each circle has ten positions.
        for (uint16_t i = 0; i < 2350; i = i + 50){
            pbf_move_right_joystick(context, {0, -1}, 40ms, 0ms);
            pbf_move_right_joystick(context, {+0.583, -0.811}, 40ms, 0ms);
            pbf_move_right_joystick(context, {+0.953, -0.307}, 40ms, 0ms);
            pbf_move_right_joystick(context, {+0.953, +0.312}, 40ms, 0ms);
            pbf_move_right_joystick(context, {+0.583, +0.812}, 40ms, 0ms);
            pbf_move_right_joystick(context, {0, +1}, 40ms, 0ms);
            pbf_move_right_joystick(context, {-0.586, +0.812}, 40ms, 0ms);
            pbf_move_right_joystick(context, {-0.953, +0.312}, 40ms, 0ms);
            pbf_move_right_joystick(context, {-0.953, -0.307}, 40ms, 0ms);
            pbf_move_right_joystick(context, {-0.586, -0.811}, 40ms, 0ms);
        }


        //  Last step for the curry cooking part.
        pbf_wait(context, 3400ms);
        pbf_press_button(context, BUTTON_A, 40ms, 18000ms);

        //  Press A when it shows your curry, and its class.
        pbf_press_button(context, BUTTON_A, 40ms, 12000ms);
        pbf_press_button(context, BUTTON_A, 40ms, 4000ms);
        // You are now back to the camping with your 6 Pokemon, and hopefully a curry Pokemon.

        {
            context.wait_for_all_requests();

            ReceivePokemonOverWatcher receive_detector(false);
//            ShinySparkleDetector shiny_detector(
//                env.console, env.console,
//                ImageFloatBox(0.1, 0.01, 0.8, 0.77)
//            );
            InferenceSession inference(
                context, env.console,
                {
                    {receive_detector},
//                    {shiny_detector},
                }
            );

            //  Default implementation of the "attract curry Pokemon" routine.
            pbf_move_left_joystick(context, {0, +1}, 320ms, 40ms);     //  Move up a bit to avoid talking to your pokemon.
            pbf_press_button(context, BUTTON_A, 40ms, 40ms);
            pbf_move_left_joystick(context, {+1, 0}, 440ms, 0ms);     //  Right
            for (uint16_t i = 0;  i< 2; i++){
                pbf_move_left_joystick(context, {-1, 0}, 216ms, 0ms); //  Left
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {-1, 0}, 216ms, 0ms); //  Left
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {-1, 0}, 216ms, 0ms); //  Left
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {-1, 0}, 216ms, 0ms); //  Left
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {+1, 0}, 216ms, 0ms); //  Right
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {+1, 0}, 216ms, 0ms); //  Right
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {+1, 0}, 216ms, 0ms); //  Right
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                pbf_move_left_joystick(context, {+1, 0}, 216ms, 0ms); //  Right
                pbf_press_button(context, BUTTON_A, 40ms, 40ms);
            }


            //  This routine gives better odds of attracting a low Pokemon if the option is enabled.
            if (SMALL_POKEMON.get() > 0ms){
                pbf_move_left_joystick(context, {0, -1}, SMALL_POKEMON, 0ms);

                for (uint16_t i = 0; i<2; i++){
                    pbf_move_left_joystick(context, {-1, 0}, 240ms, 0ms); // Left
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                    pbf_move_left_joystick(context, {-1, 0}, 240ms, 0ms); // Left
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                    pbf_move_left_joystick(context, {-1, 0}, 240ms, 0ms); // Left
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                    pbf_move_left_joystick(context, {+1, 0}, 240ms, 0ms); // Right
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                    pbf_move_left_joystick(context, {+1, 0}, 240ms, 0ms); // Right
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                    pbf_move_left_joystick(context, {+1, 0}, 240ms, 0ms); // Right
                    pbf_press_button(context, BUTTON_A, 40ms, 40ms);
                }
            
            }

            //  Give the pokemon the time to come to us.
            pbf_wait(context, WALK_UP_DELAY0);

            //  Record the encounter.
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 4000ms);
            }else{
                pbf_wait(context, 6000ms);
            }


            //  At this point, let's assume we have attracted the curry pokemon.
            //  Now we need to add it to our party while making sure this won't break anything if there is
            //  No curry encounter.
            //  This sequence seems to work because when you whistle at a curry encounter, your camera
            //  Automatically focuses on it, while it doesn't do that for your pokemons.

            //  "a [wild pokemon] came to your camp !"
            pbf_press_button(context, BUTTON_A, 40ms, 2000ms);

            //  "[wild pokemon] seems to want to come with you!"
            pbf_press_button(context, BUTTON_A, 40ms, 1600ms);

            //  "do you want to adopt it ?"
            pbf_press_button(context, BUTTON_A, 40ms, 1600ms);

            //  "you adopted [wild pokemon]!"
            pbf_press_button(context, BUTTON_A, 40ms, 1600ms);

            //  "[wild pokemon] entered into a poke ball"
            pbf_press_button(context, BUTTON_A, 40ms, 6400ms);

            //  "[wild pokemon] has been sent to the PC"
            pbf_press_button(context, BUTTON_A, 40ms, 3000ms);


            context.wait_for_all_requests();
            ShinyType shininess = ShinyType::NOT_SHINY;
            if (receive_detector.triggered()){
//                shininess = shiny_detector.results();
#if 1
                stats.add_non_shiny();
#else
                switch (shininess){
                case ShinyType::UNKNOWN:
                case ShinyType::NOT_SHINY:
                    stats.add_non_shiny();
                    break;
                case ShinyType::UNKNOWN_SHINY:
                    stats.add_unknown_shiny();
                    break;
                case ShinyType::STAR_SHINY:
                    stats.add_star_shiny();
                    break;
                case ShinyType::SQUARE_SHINY:
                    stats.add_square_shiny();
                    break;

                }
#endif
            }

            stats.m_attempts++;
            env.update_stats();
            if (shininess != ShinyType::NOT_SHINY){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 4000ms);
            }
        }


        //  If you talked to the curry Pokemon too early, you can end up talking to one of your Pokemon
        //  Once you reach this part of the program. The following sequence ensures that you
        //  Are in a correct state to cook again.
        pbf_move_left_joystick(context, {+1, 0}, 1000ms, 40ms);
        pbf_move_left_joystick(context, {0, +1}, 1000ms, 40ms);
        pbf_move_left_joystick(context, {0, +1}, 40ms, 400ms);
        pbf_press_button(context, BUTTON_A, 40ms, 2000ms);    //  Wait 2 seconds to be sure the following X press won't be dropped.

        //  And now we cook another curry.
        pbf_press_button(context, BUTTON_X, 40ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 40ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 40ms, 2400ms);
    }


    //  Not really relevant here, but for programs that finish, go to
    //  Switch home to idle.
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
}




}
}
}
