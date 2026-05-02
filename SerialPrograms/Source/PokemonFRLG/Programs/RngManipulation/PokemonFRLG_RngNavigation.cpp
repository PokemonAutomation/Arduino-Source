#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

void go_to_starter_summary(ConsoleHandle& console, ProControllerContext& context){
    // Navigate to summary (1st party slot)
    open_start_menu(console, context); // Don't have a Pokedex yet, so arrow will already by over POKeMON
    
    SummaryWatcher summary_open(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
            for (int i=0; i<3; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            }
        },
        { summary_open }
    );

    if (ret < 0){
        console.log("go_to_starter_summary(): failed to open the summary.");
    }else{
        console.log("Summary opened.");
    }
}

bool shiny_check_starter_summary(ConsoleHandle& console, ProControllerContext& context){
    go_to_starter_summary(console, context);
    context.wait_for_all_requests();
    VideoSnapshot screen = console.video().snapshot();
    ShinySymbolDetector shiny_checker(COLOR_YELLOW);
    return shiny_checker.read(console.logger(), screen);
}

void go_to_last_summary(ConsoleHandle& console, ProControllerContext& context){
    // navigate to the last occupied party slot
    open_party_menu_from_overworld(console, context);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);

    // open summary
    SummaryWatcher summary_open(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
            for (int i=0; i<3; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            }
        },
        { summary_open }
    );

    if (ret < 0){
        console.log("go_to_last_summary(): failed to open the summary.");
    } else {
        console.log("Summary opened.");
    }
}

bool shiny_check_summary(ConsoleHandle& console, ProControllerContext& context){
    go_to_last_summary(console, context);
    context.wait_for_all_requests();
    VideoSnapshot screen = console.video().snapshot();
    ShinySymbolDetector shiny_checker(COLOR_YELLOW);
    return shiny_checker.read(console.logger(), screen);
}

void hatch_togepi_egg(ConsoleHandle& console, ProControllerContext& context){
    // assumes the player is already on a bike and that the nearby trainer has been defeated
    // cycle to the right
    pbf_move_left_joystick(context, {+1, 0}, 1000ms, 200ms);
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 500ms);
    WhiteDialogWatcher egg_dialog(COLOR_RED);
    context.wait_for_all_requests();
    WallClock deadline = current_time() + 600s;
    console.log("Hatching Togepi egg...");
    int ret = run_until<ProControllerContext>(
        console, context,
        [deadline](ProControllerContext& context) {
            // cycle back and forth
            while (current_time() < deadline){
                pbf_move_left_joystick(context, {-1, 0}, 400ms, 0ms);
                pbf_move_left_joystick(context, {+1, 0}, 400ms, 0ms); 
            }
        },
        { egg_dialog }
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Togepi: failed to hatch egg within 10 minutes. Check your in-game setup.",
            console
        );
    }

    // watch hatching animation and decline nickname
    pbf_mash_button(context, BUTTON_B, 15000ms);
    context.wait_for_all_requests();
}

int watch_for_shiny_encounter(ConsoleHandle& console, ProControllerContext& context){
    BlackScreenWatcher battle_entered(COLOR_RED);
    context.wait_for_all_requests();
    console.log("Wild encounter started.");
    int ret = wait_until(
        console, context, 10000ms,
        {battle_entered}
    );
    if (ret < 0){
        // OperationFailedException::fire(
        //     ErrorReport::SEND_ERROR_REPORT,
        //     "Failed to initiate encounter.",
        //     console
        // );
        return -1;
    }
    bool encounter_shiny = handle_encounter(console, context, false);
    return encounter_shiny ? 1 : 0;
}

bool check_for_shiny(ConsoleHandle& console, ProControllerContext& context, PokemonFRLG_RngTarget TARGET){
    switch (TARGET){
    case PokemonFRLG_RngTarget::starters:
        return shiny_check_starter_summary(console, context);
    case PokemonFRLG_RngTarget::togepi:
        hatch_togepi_egg(console, context);
    case PokemonFRLG_RngTarget::magikarp:
    case PokemonFRLG_RngTarget::hitmonlee:
    case PokemonFRLG_RngTarget::hitmonchan:
    case PokemonFRLG_RngTarget::hitmon:
    case PokemonFRLG_RngTarget::eevee:
    case PokemonFRLG_RngTarget::lapras:
    case PokemonFRLG_RngTarget::omanyte:
    case PokemonFRLG_RngTarget::kabuto:
    case PokemonFRLG_RngTarget::aerodactyl:
    case PokemonFRLG_RngTarget::fossils:
    case PokemonFRLG_RngTarget::gamecornerabra:
    case PokemonFRLG_RngTarget::gamecornerclefairy:
    case PokemonFRLG_RngTarget::gamecornerdratini:
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
    case PokemonFRLG_RngTarget::gamecornerbug:
    case PokemonFRLG_RngTarget::gamecornerporygon:
        return shiny_check_summary(console, context);
    case PokemonFRLG_RngTarget::electrode:
    case PokemonFRLG_RngTarget::articuno:
    case PokemonFRLG_RngTarget::zapdos:
    case PokemonFRLG_RngTarget::moltres:
    case PokemonFRLG_RngTarget::lugia:
    case PokemonFRLG_RngTarget::deoxys_attack:
    case PokemonFRLG_RngTarget::deoxys_defense:
    case PokemonFRLG_RngTarget::staticencounter:
    case PokemonFRLG_RngTarget::snorlax:
    case PokemonFRLG_RngTarget::mewtwo:
    case PokemonFRLG_RngTarget::hooh:
    case PokemonFRLG_RngTarget::hypno:
    case PokemonFRLG_RngTarget::sweetscent:
    case PokemonFRLG_RngTarget::fishing:
    case PokemonFRLG_RngTarget::safarizonecenter:
    case PokemonFRLG_RngTarget::safarizoneeast:
    case PokemonFRLG_RngTarget::safarizonenorth:
    case PokemonFRLG_RngTarget::safarizonewest:
    case PokemonFRLG_RngTarget::safarizonesurf:
    case PokemonFRLG_RngTarget::safarizonefish:
        return watch_for_shiny_encounter(console, context) == 1;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Option not yet implemented.",
            console
        );
    }
}

}
}
}