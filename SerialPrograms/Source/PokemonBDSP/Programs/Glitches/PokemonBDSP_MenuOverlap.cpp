/*  Menu Overlap Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP_MenuOverlap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


QImage activate_menu_overlap_from_overworld(ConsoleHandle& console){
    //  Capture screenshot.
    console.botbase().wait_for_all_requests();
    QImage start = console.video().snapshot();

    //  Enter Summary
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(console, BUTTON_ZL, 20, GameSettings::instance().MENU_TO_POKEMON_DELAY);
    pbf_press_button(console, BUTTON_ZL, 20, 50);
    pbf_press_button(console, BUTTON_ZL, 20, 250);

    //  Activate overlap.
    pbf_press_button(console, BUTTON_ZL, 10, 0);
    pbf_press_button(console, BUTTON_ZR, 20, 250);

    //  Back out.
    if (back_out_to_overworld_with_overlap(console, start, 4 * TICKS_PER_SECOND)){
        return start;
    }

    return QImage();
}


bool back_out_to_overworld_with_overlap(
    ConsoleHandle& console,
    const QImage& start,
    uint16_t mash_B_start
){
    console.log("Backing out to overworld with overlap...");
    pbf_mash_button(console, BUTTON_B, mash_B_start);

    const double THRESHOLD = 50;
    const std::chrono::milliseconds HOLD_DURATION(200);

    VideoOverlaySet boxes(console);
    ImageMatchWatcher background_all(start, {0, 0, 1, 1}, THRESHOLD, false, HOLD_DURATION);
    ImageMatchWatcher background_left(start, {0.02, 0.2, 0.08, 0.5}, THRESHOLD, false, HOLD_DURATION);
    ImageMatchWatcher background_right(start, {0.90, 0.2, 0.08, 0.5}, THRESHOLD, false, HOLD_DURATION);
    background_all.make_overlays(boxes);
    background_left.make_overlays(boxes);
    background_right.make_overlays(boxes);
    size_t backouts = 0;
    while (true){
        pbf_press_button(console, BUTTON_B, 20, 205);
        console.botbase().wait_for_all_requests();
        backouts++;
        if (backouts > 15){
            console.log("Overworld not detected after backing out 16 times.", COLOR_RED);
            return false;
        }

        QImage current = console.video().snapshot();
        double rmsd_left = background_left.rmsd(current);
        console.log("Left Background RMSD = " + std::to_string(rmsd_left));
        double rmsd_right = background_right.rmsd(current);
        console.log("Right Background RMSD = " + std::to_string(rmsd_right));
        if (rmsd_left < THRESHOLD || rmsd_right < THRESHOLD){
            break;
        }
    }

    QImage current = console.video().snapshot();
    double rmsd_screen = background_all.rmsd(current);
    if (rmsd_screen < THRESHOLD){
        console.log("Backed all the way out to overworld. Failed to activate menu overlap glitch.", COLOR_RED);
        return false;
    }

    return true;
}

void back_out_to_overworld(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    const QImage& start
){
    console.log("Backing out to overworld...");
    const double THRESHOLD = 50;
    const std::chrono::milliseconds HOLD_DURATION(200);

    VideoOverlaySet boxes(console);
    ImageMatchWatcher background_all(start, {0, 0, 1, 1}, THRESHOLD, false, HOLD_DURATION);
    background_all.make_overlays(boxes);

    int ret = run_until(
        env, context, console,
        [](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 20 * TICKS_PER_SECOND);
        },
        { &background_all }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Failed to back out to overworld in 20 seconds. Something is wrong.");
    }
    console.log("Overworld detected.");
    pbf_mash_button(console, BUTTON_B, 250);
}




}
}
}
