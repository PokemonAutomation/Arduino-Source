/*  Poffin Cooker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP_PoffinCooker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;


PoffinCooker_Descriptor::PoffinCooker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:PoffinCooker",
        STRING_POKEMON + " BDSP", "Poffin Cooker",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/PoffinCooker.md",
        "Cook Poffins.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct PoffinCooker_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Poffins cooked"])
    {
        m_display_order.emplace_back("Poffins cooked");
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> PoffinCooker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


PoffinCooker::PoffinCooker()
    : GO_HOME_WHEN_DONE(false)
    , MAX_COOK_ATTEMPTS(
        "<b>Cook this many times:</b><br>This puts a limit on how many poffins you get. Don't forget that each cooking session gets you 4 poffins, and your bag cannot have more than 100 poffins. Thus you should never input more than 25 here.",
        LockMode::LOCK_WHILE_RUNNING,
        1,
        1,
        25
    )

{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(MAX_COOK_ATTEMPTS);
}

bool turn = true; // True to turn clockwise, false to turn counter-clockwise

ImageFloatBox box(0.56, 0.724, 0.012, 0.024); // Create a box that contains both green and blue arrows that need to be detected

void TurnClockwiseSlow(ProControllerContext& context){ // One turn of stirring poffin at slow speed (clockwise)
    pbf_move_right_joystick(context, 128, 255, 5, 0);
    pbf_move_right_joystick(context, 53, 231, 5, 0);
    pbf_move_right_joystick(context, 6, 167, 5, 0);
    pbf_move_right_joystick(context, 6, 88, 5, 0);
    pbf_move_right_joystick(context, 53, 24, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_move_right_joystick(context, 202, 24, 5, 0);
    pbf_move_right_joystick(context, 249, 88, 5, 0);
    pbf_move_right_joystick(context, 249, 167, 5, 0);
    pbf_move_right_joystick(context, 202, 231, 5, 0);  
}

void TurnClockwiseFast(ProControllerContext& context){ // Same as above, but faster for the end of the cooking session
    pbf_move_right_joystick(context, 128, 255, 5, 0);
    pbf_move_right_joystick(context, 38, 218, 5, 0);
    pbf_move_right_joystick(context, 0, 128, 5, 0);
    pbf_move_right_joystick(context, 38, 38, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_move_right_joystick(context, 218, 38, 5, 0);
    pbf_move_right_joystick(context, 255, 128, 5, 0);
    pbf_move_right_joystick(context, 218, 218, 5, 0); 
}


void TurnCounterClockwiseSlow(ProControllerContext& context){ // One turn of stirring poffin (counter-clockwise)
    pbf_move_right_joystick(context, 128, 255, 5, 0);
    pbf_move_right_joystick(context, 202, 231, 5, 0);
    pbf_move_right_joystick(context, 249, 167, 5, 0);
    pbf_move_right_joystick(context, 249, 88, 5, 0);
    pbf_move_right_joystick(context, 202, 24, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_move_right_joystick(context, 53, 24, 5, 0);
    pbf_move_right_joystick(context, 6, 88, 5, 0);
    pbf_move_right_joystick(context, 6, 167, 5, 0);
    pbf_move_right_joystick(context, 53, 231, 5, 0);
}

void TurnCounterClockwiseFast(ProControllerContext& context){ // Same as above, but faster for the end of the cooking session
    pbf_move_right_joystick(context, 128, 255, 5, 0);
    pbf_move_right_joystick(context, 218, 218, 5, 0); 
    pbf_move_right_joystick(context, 255, 128, 5, 0);
    pbf_move_right_joystick(context, 218, 38, 5, 0);
    pbf_move_right_joystick(context, 128, 0, 5, 0);
    pbf_move_right_joystick(context, 38, 38, 5, 0);
    pbf_move_right_joystick(context, 0, 128, 5, 0);
    pbf_move_right_joystick(context, 38, 218, 5, 0); 
}


void PoffinCooker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    PoffinCooker_Descriptor::Stats& stats = env.current_stats<PoffinCooker_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    env.log("Select the cooking option");
    //  Select the cooking option.
    pbf_press_button(context, BUTTON_A, 5, 100);
    pbf_press_button(context, BUTTON_A, 5, 175);
    pbf_press_button(context, BUTTON_A, 5, 75);
    pbf_press_button(context, BUTTON_A, 5, 100);


    for (uint16_t c = 0; c < MAX_COOK_ATTEMPTS; c++){

        env.log("Select the 4 berries to use");
        // Select the first four berries to cook and confirm the selection.
        pbf_press_button(context, BUTTON_A, 5, 50);
        pbf_press_dpad(context, DPAD_DOWN, 5, 50);
        pbf_press_button(context, BUTTON_A, 5, 50);
        pbf_press_dpad(context, DPAD_DOWN, 5, 50);        
        pbf_press_button(context, BUTTON_A, 5, 50);
        pbf_press_dpad(context, DPAD_DOWN, 5, 50);
        pbf_press_button(context, BUTTON_A, 5, 50);
        pbf_press_button(context, BUTTON_A, 5, 50);
        pbf_mash_button(context, BUTTON_A, 150); // Mash here to make sure the final button press isn't dropped

        // Wait a bit less than 10 seconds for the cinematic to happen then cook.
        pbf_wait(context, 1050);
        context.wait_for_all_requests();

        env.log("Stir slowly for the first part");
        for (uint16_t d = 0; d < 79; d++){
            // Capture the image on the screen
            VideoSnapshot screen = env.console.video().snapshot();

            // Get the stats of the screen's image
            ImageStats IMGstats = image_stats(extract_box_reference(screen, box));
            if (IMGstats.average.g > 170 && IMGstats.average.r < 125) { // Looking for the green arrow
                turn = true;
            }
            if (IMGstats.average.b > 170 && IMGstats.average.r < 125) { // Looking for the blue arrow
                turn = false;
            }
            if (turn){
                TurnClockwiseSlow(context);
            }else{
                TurnCounterClockwiseSlow(context);
            }
        }

        env.log("Stir at full speed now");
        for (uint16_t d = 0; d < 70; d++){
            // Capture the image on the screen
            VideoSnapshot screen = env.console.video().snapshot();

            // Get the stats of the screen's image
            ImageStats IMGstats = image_stats(extract_box_reference(screen, box));
            if (IMGstats.average.g > 170 && IMGstats.average.r < 125) { // Looking for the green arrow
                turn = true;
            }
            if (IMGstats.average.b > 170 && IMGstats.average.r < 125) { // Looking for the blue arrow
                turn = false;
            }
            if (turn){
                TurnClockwiseFast(context);
            }else{
                TurnCounterClockwiseFast(context);
            }
        }

        // Final animation when the cooking session is over
        pbf_wait(context, 750);

        if (c < MAX_COOK_ATTEMPTS - 1){
            env.log("Prepare for the next iteration");
            pbf_press_button(context, BUTTON_A, 5, 125);
            pbf_press_button(context, BUTTON_A, 5, 125);
            pbf_press_button(context, BUTTON_A, 5, 125);                
        }
        stats.m_attempts++;
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
