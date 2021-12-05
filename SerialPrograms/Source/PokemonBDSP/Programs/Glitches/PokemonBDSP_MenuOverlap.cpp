/*  Menu Overlap Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
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
    pbf_press_button(console, BUTTON_X, 10, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);
    pbf_press_button(console, BUTTON_A, 10, GameSettings::instance().MENU_TO_POKEMON_DELAY);
    pbf_press_button(console, BUTTON_A, 10, 50);
    pbf_press_button(console, BUTTON_A, 10, 250);

    //  Activate overlap.
    pbf_press_button(console, BUTTON_ZL, 5, 3);
    pbf_press_button(console, BUTTON_A, 5, 250);

    //  Back out.
    if (back_out_to_overworld(console, start, 4 * TICKS_PER_SECOND)){
        return start;
    }

    return QImage();
}


bool back_out_to_overworld(
    ConsoleHandle& console,
    const QImage& start,
    uint16_t mash_B_start
){
    console.log("Backing out to overworld...");
    pbf_mash_button(console, BUTTON_B, mash_B_start);

    const double THRESHOLD = 100;

    std::deque<InferenceBoxScope> boxes;
    ImageMatchDetector background_left(start, {0.02, 0.2, 0.08, 0.5}, THRESHOLD);
    ImageMatchDetector background_right(start, {0.90, 0.2, 0.08, 0.5}, THRESHOLD);
    background_left.make_overlays(boxes, console);
    background_right.make_overlays(boxes, console);
    size_t backouts = 0;
    while (true){
        pbf_press_button(console, BUTTON_B, 10, 215);
        console.botbase().wait_for_all_requests();
        backouts++;
        if (backouts > 15){
            console.log("Overworld not detected after backing out 16 times.", Qt::red);
            return false;
        }

        QImage current = console.video().snapshot();
        double rmsd_left = background_left.rmsd(current);
        console.log("Left Background RMSD = " + std::to_string(rmsd_left));
        double rmsd_right = background_right.rmsd(current);
        console.log("Right Background RMSD = " + std::to_string(rmsd_right));
        if (rmsd_left < THRESHOLD || rmsd_right < THRESHOLD){
            return true;
        }
    }
}


}
}
}
