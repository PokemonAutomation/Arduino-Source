#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_PartyDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_SummaryDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_BagDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_DexRegistrationDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PartyLevelUpReader.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngCalibration.h"
#include "PokemonFRLG_RngNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;

void go_to_summary(ConsoleHandle& console, ProControllerContext& context, int from_last, StartMenuContext menu_context){
    // navigate to the specified party slot
    open_party_menu_from_overworld(console, context, menu_context);
    for (int i=0; i<(2+from_last); i++){
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }

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
        console.log("go_to_summary(): failed to open the summary.");
    } else {
        console.log("Summary opened.");
    }
}

bool shiny_check_summary(ConsoleHandle& console, ProControllerContext& context, int from_last, StartMenuContext menu_context){
    go_to_summary(console, context, from_last, menu_context);
    context.wait_for_all_requests();
    VideoSnapshot screen = console.video().snapshot();
    ShinySymbolDetector shiny_checker(COLOR_YELLOW);
    return shiny_checker.read(console.logger(), screen);
}

AdvObservedPokemon read_summary(
    ConsoleHandle& console, ProControllerContext& context, 
    const Language& language, std::set<std::string> species
){
    // read stats
    PokemonFRLG_Stats stats;
    StatsReader reader(COLOR_RED);

    console.log("Reading Page 1 (Name, Level, Nature, Gender)...");
    VideoSnapshot screen1 = console.video().snapshot();
    reader.read_page1(console.logger(), language, screen1, stats, species);

    SummaryPage2Watcher page_two(COLOR_RED);
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_dpad(context, DPAD_RIGHT, 200ms, 1800ms);
            }
        },
        { page_two }
    );

    if (ret2 < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "read_summary(): Failed to detect second summary screen.",
            console
        ); 
    }

    console.log("Reading Page 2 (Stats)...");
    VideoSnapshot screen2 = console.video().snapshot();
    reader.read_page2(console.logger(), language, screen2, stats);

    StatReads statreads = {
        static_cast<int16_t>(stats.hp.value_or(0)),
        static_cast<int16_t>(stats.attack.value_or(0)),
        static_cast<int16_t>(stats.defense.value_or(0)),
        static_cast<int16_t>(stats.sp_attack.value_or(0)),
        static_cast<int16_t>(stats.sp_defense.value_or(0)),
        static_cast<int16_t>(stats.speed.value_or(0))
    };

    AdvGender gender;
    switch(stats.gender.value_or(SummaryGender::Genderless)){
    case SummaryGender::Male:
        gender = AdvGender::Male;
        break;
    case SummaryGender::Female:
        gender = AdvGender::Female;
        break;
    default:
        gender = AdvGender::Any;
        break;
    }

    AdvObservedPokemon pokemon = {
        stats.name,
        gender,
        string_to_nature(stats.nature),
        AdvAbility::Any,
        { uint8_t(stats.level.value_or(5)) },
        { statreads },
        { {0,0,0,0,0,0} },
        AdvShinyType::Any
    };

    return pokemon;
}


int auto_catch(
    ConsoleHandle& console, 
    ProControllerContext& context, 
    const uint64_t& max_ball_throws,
    bool safari_zone
){
    for (uint64_t i=0; i<=max_ball_throws; i++){
        int count = 0;
        while(true){
            if (count >= 10){
                console.log("auto_catch(): failed to detect battle menu");
                return -1;
            }
            count++;

            BattleMenuWatcher battle_menu(COLOR_RED);
            PartyMenuWatcher party_menu(COLOR_RED);
            DexRegistrationWatcher dex_registration(COLOR_RED);
            BlackScreenWatcher black_screen(COLOR_RED);
            context.wait_for_all_requests();
            int ret = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<60; i++){
                        pbf_press_button(context, BUTTON_B, 200ms, 300ms);
                    }
                },
                { battle_menu, party_menu, black_screen },
                10ms
            );

            int start_ret;
            switch (ret){
            case 0:
                console.log("Battle menu detected");
                break;
            case 1:
                console.log("Party menu detected. Attempting to send out next Pokemon");
                pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
                pbf_mash_button(context, BUTTON_A, 1000ms);
                continue;
            case 2:
                console.log("Dex registration detected. Exiting battle...");
                pbf_mash_button(context, BUTTON_B, 5000ms);
                return static_cast<int>(i);
            case 3:
                console.log("Black screen detected. Battle exited.");
                return static_cast<int>(i);
            default:
                console.log("No recognized state. Try checking if in the overworld...");
                StartMenuWatcher start_menu;
                context.wait_for_all_requests();
                start_ret = run_until<ProControllerContext>(
                    console, context,
                    [](ProControllerContext& context) {
                        for (int i=0; i<3; i++){
                            pbf_press_button(context, BUTTON_PLUS, 200ms, 2800ms);
                            pbf_mash_button(context, BUTTON_B, 500ms);
                        }
                    },
                    { start_menu }
                );
                if (start_ret < 0){
                    console.log("auto_catch(): no recognized state after 30 seconds."); 
                    return true;
                }
                console.log("Overworld detected.");
                pbf_mash_button(context, BUTTON_B, 500ms);
                context.wait_for_all_requests();
                return static_cast<int>(i);
            }

            break;
        }

        if (i == max_ball_throws) { break; }

        if (!safari_zone){
            // select BAG (selection arrow does not wrap around)
            pbf_move_left_joystick(context, {+1, 0}, 100ms, 150ms);
            pbf_move_left_joystick(context, {0, +1}, 100ms, 150ms);
            pbf_move_left_joystick(context, {+1, 0}, 100ms, 150ms);
            pbf_move_left_joystick(context, {0, +1}, 100ms, 150ms);

            BagWatcher bag_open(COLOR_RED);
            int ret2 = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<5; i++){
                        pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
                    }
                },
                { bag_open }
            );
            if (ret2 < 0){
                console.log("auto_catch(): failed to open bag."); 
                return -1;
            }

            if (i == 0){
                // go to balls pocket (pockets do not wrap around, topmost item will already be selected)
                pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
                pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
                pbf_move_left_joystick(context, {+1, 0}, 200ms, 800ms);
            }
        }

        // use ball
        pbf_mash_button(context, BUTTON_A, 5s);
    }

    console.log("auto_catch(): ran out of balls.");
    return 0;
}

bool use_rare_candy(
    ConsoleHandle& console, 
    ProControllerContext& context,
    const Language& language, 
    AdvObservedPokemon& pokemon,
    AdvRngFilters& filters,
    const BaseStats& base_stats,
    AdvRngMethod method,
    bool safari_zone,
    bool first
){
    // navigate to the bag (only needed for the first use)
    if (first){
        open_bag_from_overworld(console, context, safari_zone ? PokemonFRLG::StartMenuContext::SAFARI_ZONE : PokemonFRLG::StartMenuContext::STANDARD);
        // move left to the correct pocket (in case Teachy TV was used)
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
    }

    // use rare candy and watch for the party screen
    PartyMenuWatcher party_menu(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<5; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            }
        },
        { party_menu }
    );
    if (ret < 0){
        console.log("use_rare_candy(): failed to detect party menu."); 
        return true;
    }

    // select the last party slot (unknown how full the party is, so we can't detect a particular slot)
    // only needed on the first use
    if (first){
        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }

    // watch for level up stats
    PartyLevelUpWatcher level_up(COLOR_RED, PartyLevelUpDialog::stats, language);
    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            for (int i=0; i<30; i++){
                pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            }
        },
        { level_up }
    );
    if (ret2 < 0){
        console.log("use_rare_candy(): failed to detect level-up stats."); 
        return true;
    }

    PartyLevelUpReader reader(COLOR_RED);
    VideoOverlaySet overlays(console.overlay());
    reader.make_overlays(overlays);

    console.log("Reading stats...");
    VideoSnapshot screen = console.video().snapshot();
    StatReads statreads = reader.read_stats(console.logger(), screen);    

    update_filters(filters, pokemon, statreads, {}, base_stats, method);
    // RNG_FILTERS.set(filters);   

    // return to the bag (possibly learning a move, but trying to prevent evolution)
    int attempts = 0;
    while (true){
        if (attempts > 5){
            console.log("use_rare_candy(): failed to return to bag menu in 5 attempts.");
            return true;
        }
        BagWatcher bag_menu(COLOR_RED);
        PartyMoveLearnWatcher move_learn(COLOR_RED);
        context.wait_for_all_requests();
        int ret3 = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                for (int i=0; i<15; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
                }
            },
            { bag_menu, move_learn }
        );
        attempts++;
        switch (ret3){
        case 0:
            console.log("Returned to bag.");
            return false;
        case 1:
            console.log("Move learn opportunity detected.");
            // don't learn move
            pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            continue;
        default:
            console.log("use_rare_candy(): failed to return to bag menu.");
            return true;
        }
    }
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

void hatch_daycare_egg(ConsoleHandle& console, ProControllerContext& context){
    // assumes the player is already on a bike
    pbf_move_left_joystick(context, {-1, 0}, 1000ms, 500ms);
    pbf_move_left_joystick(context, {+1, 0}, 60ms, 440ms);
    WhiteDialogWatcher egg_dialog(COLOR_RED);
    context.wait_for_all_requests();
    WallClock deadline = current_time() + 2700s;
    console.log("Hatching Day Care egg...");
    int ret = run_until<ProControllerContext>(
        console, context,
        [deadline](ProControllerContext& context) {
            // cycle back and forth
            while (current_time() < deadline){
                pbf_move_left_joystick(context, {+1, 0}, 1700ms, 30ms);
                pbf_move_left_joystick(context, {-1, 0}, 1700ms, 30ms); 
            }
        },
        { egg_dialog }
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Daycare Egg: failed to hatch egg within 45 minutes. Check your in-game setup.",
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
    console.log("Wild encounter started.");
    bool encounter_shiny = handle_encounter(console, context, false);
    return encounter_shiny ? 1 : 0;
}

bool check_for_shiny(ConsoleHandle& console, ProControllerContext& context, PokemonFRLG_RngTarget TARGET){
    switch (TARGET){
    case PokemonFRLG_RngTarget::eggheld:
        return false;
    case PokemonFRLG_RngTarget::eggpickup:
        hatch_daycare_egg(console, context);
        return shiny_check_summary(console, context);
    case PokemonFRLG_RngTarget::starters:
        return shiny_check_summary(console, context, -2, StartMenuContext::NO_DEX);
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
    case PokemonFRLG_RngTarget::gamecornerdratinifr:
    case PokemonFRLG_RngTarget::gamecornerdratinilg:
    case PokemonFRLG_RngTarget::gamecornerscyther:
    case PokemonFRLG_RngTarget::gamecornerpinsir:
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
    case PokemonFRLG_RngTarget::rocksmash:
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
