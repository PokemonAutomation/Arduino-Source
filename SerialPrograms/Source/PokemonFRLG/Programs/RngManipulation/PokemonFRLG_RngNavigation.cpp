#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_CatchFanfareDetector.h"
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
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PokemonSpriteReader.h"
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
    
    if (
           stats.name.empty()
        || stats.name == "nidoran-m"
        || stats.name == "nidoran-f"
    ){
        SummarySpriteReader sprite_reader(species);
        ImageMatch::ImageMatchResult result = sprite_reader.read(screen1);
        if (result.results.size() > 0){
            stats.name = result.results.begin()->second;
            console.log("Matched sprite: " + stats.name);
        }
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
    float catch_coefficient = 1.0;
    bool catch_detected = false;

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
            CatchFanfareDetector catch_detector(console.logger(), [&](float error_coefficient) -> bool{
                catch_coefficient = error_coefficient;
                return true;
            });
            context.wait_for_all_requests();
            int ret = run_until<ProControllerContext>(
                console, context,
                [](ProControllerContext& context) {
                    for (int i=0; i<60; i++){
                        pbf_press_button(context, BUTTON_B, 200ms, 300ms);
                    }
                },
                { battle_menu, party_menu, dex_registration, black_screen, catch_detector},
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
                return catch_detected ? static_cast<int>(i) : 0;
            case 3:
                console.log("Black screen detected. Battle exited.");
                pbf_mash_button(context, BUTTON_B, 2500ms);
                return catch_detected ? static_cast<int>(i) : 0;
            case 4: 
                console.log("Catch detected!", COLOR_BLUE);
                catch_detected = true;
                pbf_wait(context, 2000ms);
                continue;
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
                return catch_detected ? static_cast<int>(i) : 0;
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
    bool first,
    int from_last
){
    // navigate to the bag (only needed for the first use)
    if (first){
        open_bag_from_overworld(console, context, safari_zone ? PokemonFRLG::StartMenuContext::SAFARI_ZONE : PokemonFRLG::StartMenuContext::STANDARD);
        // move left to the correct pocket (in case Teachy TV was used)
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {-1, 0}, 200ms, 800ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
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
        for (int i=0; i<(2+from_last); i++){
            pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        }
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

    // try to exit the party menu before using the BattleDialogueWatcher,
    // since the fading party screen can trigger it.
    pbf_press_button(context, BUTTON_B, 200ms, 1800ms);

    update_filters(filters, pokemon, statreads, {}, base_stats, method);

    // return to the bag (possibly learning a move, but trying to prevent evolution)
    int attempts = 0;
    while (true){
        if (attempts > 25){
            console.log("use_rare_candy(): failed to return to bag menu.");
            return true;
        }
        BagWatcher bag_menu(COLOR_RED);
        PartyMoveLearnWatcher move_learn(COLOR_RED);
        BattleDialogWatcher evolution(COLOR_RED);
        context.wait_for_all_requests();
        int ret3 = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context) {
                pbf_wait(context, 1000ms);
                for (int i=0; i<15; i++){
                    pbf_press_button(context, BUTTON_B, 200ms, 1800ms);
                }
            },
            { bag_menu, move_learn, evolution }
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
            pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
            continue;
        case 2:
            console.log("Evolution screen detected.");
            pbf_press_button(context, BUTTON_B, 200ms, 2800ms);
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


void travel_from_celio_to_kanto(ConsoleHandle& console, ProControllerContext& context){
    // assumes the player is standing on Celio's left (west) side
    pbf_move_left_joystick(context, {-1, 0}, 1280ms, 300ms);
    leave_pokecenter(console, context);
    // walk down to the One Island sign
    WhiteDialogWatcher dialog_detected(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            // walk down
            pbf_move_left_joystick(context, {0, -1}, 15000ms, 0ms);

        },
        { dialog_detected }
    );
    if (ret < 0){        
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "travel_from_celio_to_route2(): Failed to detect One Island sign.",
            console
        ); 
    }

    // walk to the ferry and take it to Vermilion
    pbf_wait(context, 500ms);
    pbf_move_left_joystick(context, {-1, 0 }, 430ms, 300ms);

    context.wait_for_all_requests();
    int ret2 = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            // walk down to the sailor and initiate dialog
            ssf_press_left_joystick(context, {0, -1}, 0ms, 20000ms);
            ssf_mash1_button(context, BUTTON_A, 20000ms);
        },
        { dialog_detected }
    );
    if (ret2 < 0){        
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "travel_from_celio_to_route2(): Failed to initiate Seagallop ferry dialogue.",
            console
        ); 
    }

    BlackScreenWatcher black_screen(COLOR_RED);
    context.wait_for_all_requests();
    int ret3 = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            // select vermilion (default option) and take ferry
            pbf_mash_button(context, BUTTON_A, 20000ms);
        },
        { black_screen }
    );
    if (ret3 < 0){        
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "travel_from_celio_to_kanto(): Failed to initiate Seagallop ferry travel.",
            console
        );
    }

    // fly to pewter city
    pbf_wait(context, 6000ms);
    context.wait_for_all_requests();
    console.log("Arrived at the Vermilion docks.");
}

void travel_to_route1(ConsoleHandle& console, ProControllerContext& context){
    open_fly_map_from_overworld(console, context);
    fly_from_kanto_map(console, context, KantoFlyLocation::pallettown);

    // walk to the boundary of route 1
    pbf_move_left_joystick(context, {+1, 0}, 1370ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 1450ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 300ms,  300ms);
    pbf_move_left_joystick(context, {0, +1}, 800ms, 300ms);

    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    console.log("Arrived at Route 1.");
}

void use_repel(ConsoleHandle& console, ProControllerContext& context){
    open_bag_from_overworld(console, context);
    // move back to the Items pocket in case Teachy TV was used
    pbf_move_left_joystick(context, {-1, 0}, 500ms, 500ms);
    pbf_move_left_joystick(context, {-1, 0}, 500ms, 500ms);
    // Repel is required to be in the 2nd position (Rare Candy is first)
    for (int i=0; i<4; i++){
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    }
    pbf_wait(context, 500ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_mash_button(context, BUTTON_A, 2000ms);
    // exit to overworld
    pbf_mash_button(context, BUTTON_B, 5000ms);
    context.wait_for_all_requests();
    console.log("Used Repel.");
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

int encounter_roamer(
    ConsoleHandle& console, ProControllerContext& context, 
    Language language, const std::set<std::string>& subset
){
    travel_from_celio_to_kanto(console, context);
    int attempts = 0;
    while (true){
        if (attempts >= 250){
            console.log("encounter_roamer(): Failed to encounter roamer in 250 attempts.");
            return -1;
        }
        if ((attempts % 5) == 0){
            travel_to_route1(console, context);
        }
        if (attempts == 0){
            use_repel(console, context);
        }

        context.wait_for_all_requests();
        pbf_move_left_joystick(context, {0, +1}, 200ms, 800ms);
        int ret = grass_spin(console, context, true, 5s);
        if (ret < 0){
            attempts++;
            pbf_move_left_joystick(context, {0, -1}, 200ms, 800ms);
            continue;
        }else if (ret == 1){
            return ret;
        }else{
            WildEncounterReader reader(COLOR_RED);
            VideoOverlaySet overlays(console.overlay());
            reader.make_overlays(overlays);
            console.log("Reading name...");
            VideoSnapshot screen = console.video().snapshot();
            PokemonFRLG_WildEncounter encounter = reader.read_encounter(console.logger(), language, screen, subset);
            console.log("Name: " + encounter.name);

            if (encounter.name == "pidgey" || encounter.name == "rattata"){
                flee_battle(console, context);
                use_repel(console, context);
                pbf_move_left_joystick(context, {0, -1}, 200ms, 800ms);
                continue;
            }

            return ret; // 0
        }
    }
}

bool check_for_shiny(
    ConsoleHandle& console, ProControllerContext& context, 
    PokemonFRLG_RngTarget TARGET,
    Language language, const std::set<std::string>& subset
){
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
    case PokemonFRLG_RngTarget::raikou:
    case PokemonFRLG_RngTarget::entei:
    case PokemonFRLG_RngTarget::suicune:
    case PokemonFRLG_RngTarget::roaming:
        return encounter_roamer(console, context, language, subset) == 1;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "RNG target not recognized. Please report this as a bug.",
            console
        );
    }
}


void use_max_repel(ConsoleHandle& console, ProControllerContext& context){
    console.log("Using a Max Repel (2nd bag slot from top)...");
    open_bag_from_overworld(console, context);
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
    pbf_move_left_joystick(context, {-1, 0}, 100ms, 900ms);
    for (int i=0; i<20; i++){
        pbf_move_left_joystick(context, {0, +1}, 100ms, 100ms);
    }
    pbf_move_left_joystick(context, {0, -1}, 100ms, 300ms);

    pbf_mash_button(context, BUTTON_A, 3000ms);
    pbf_mash_button(context, BUTTON_B, 5000ms);

}

void daycare_steps(ConsoleHandle& console, ProControllerContext& context){
    console.log("Taking 250 steps...");
    // walk down to the south wall
    pbf_move_left_joystick(context, {0, -1}, 930ms, 570ms);
    // walk to the southeast corner
    pbf_move_left_joystick(context, {+1, 0}, 2500ms, 300ms);

    WhiteDialogWatcher repel_over(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
            for (int i=0; i<20; i++){
                pbf_move_left_joystick(context, {-1, 0}, 2700ms, 300ms);
                pbf_move_left_joystick(context, {+1, 0}, 2700ms, 300ms);
            }
        },
        { repel_over }
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "daycare_steps(): No Max Repel dialogue box detected.",
            console
        );
    }

    console.log("Max Repel wore off. Taking 4 more steps...");
    pbf_mash_button(context, BUTTON_B, 1000ms);

    // take 4 steps to the left
    pbf_move_left_joystick(context, {-1, 0}, 200ms, 720ms);
    pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);
    pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);
    pbf_move_left_joystick(context, {-1, 0}, 30ms, 720ms);

    console.log("254 steps taken.");
}

bool walk_from_daycare_to_pond(ConsoleHandle& console, ProControllerContext& context){
    console.log("Walking to the daycare pond...");
    pbf_move_left_joystick(context, {0, -1}, 190ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 2470ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 380ms, 300ms);

    // start surfing
    WhiteDialogWatcher surf_dialog(COLOR_RED);

    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        },
        { surf_dialog }
    );
    if (ret < 0){
        console.log("Failed to detect surf dialog");
        return true;
    }
    console.log("Started surfing.");
    pbf_mash_button(context, BUTTON_A, 2000ms);
    context.wait_for_all_requests();
    return false;
}

void walk_from_pond_to_daycare_man(ConsoleHandle& console, ProControllerContext& context){
    console.log("Walking to the daycare man...");
    pbf_move_left_joystick(context, {0, +1}, 1080ms, 300ms);
    pbf_move_left_joystick(context, {-1, 0}, 1300ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 300ms, 300ms);
}

void egg_pickup(ConsoleHandle& console, ProControllerContext& context){
    console.log("Picking up egg...");
    WhiteDialogWatcher dialogue(COLOR_RED);
    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            pbf_mash_button(context, BUTTON_A, 5000ms);
        },
        { dialogue }
    );
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "egg_pickup(): Failed to initiate dialogue.",
            console
        );
    }

    pbf_mash_button(context, BUTTON_A, 5000ms);
    pbf_mash_button(context, BUTTON_B, 2500ms);
}

bool walk_from_daycare_man_to_pond(ConsoleHandle& console, ProControllerContext& context){
    console.log("Walking to the daycare man...");
    pbf_move_left_joystick(context, {0, -1}, 380ms, 300ms);
    pbf_move_left_joystick(context, {+1, 0}, 1340ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 300ms, 300ms);

    // start surfing
    WhiteDialogWatcher surf_dialog(COLOR_RED);

    context.wait_for_all_requests();
    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            ssf_press_left_joystick(context, {0, -1}, 0ms, 10000ms);
            ssf_mash1_button(context, BUTTON_A, 10000ms);
        },
        { surf_dialog }
    );
    if (ret < 0){
        console.log("Failed to detect surf dialog");
        return true;
    }
    console.log("Started surfing.");
    pbf_mash_button(context, BUTTON_A, 2000ms);
    context.wait_for_all_requests();
    return false;
}

}
}
}
