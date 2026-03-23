/*  EV Trainer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include <map>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_EvTrainer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

EvTrainer_Descriptor::EvTrainer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:EvTrainer",
        Pokemon::STRING_POKEMON + " FRLG", "EV Trainer",
        "Programs/PokemonFRLG/EvTrainer.html",
        "Defeats wild encounters to train EVs.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct EvTrainer_Descriptor::Stats : public StatsTracker{
    Stats()
        : encounters(m_stats["Encounters"])
        , healing_trips(m_stats["Healing Trips"])
        , times_fainted(m_stats["Times Fainted"])
        , hp_evs(m_stats["HP EVs"])
        , atk_evs(m_stats["Attack EVs"])
        , def_evs(m_stats["Defense EVs"])
        , spa_evs(m_stats["Sp. Attack EVs"])
        , spd_evs(m_stats["Sp. Defense EVs"])
        , spe_evs(m_stats["Speed EVs"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Healing Trips");
        m_display_order.emplace_back("Times Fainted", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("HP EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Attack EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Defense EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Sp. Attack EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Sp. Defense EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Speed EVs", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& healing_trips;
    std::atomic<uint64_t>& times_fainted;
    std::atomic<uint64_t>& hp_evs;
    std::atomic<uint64_t>& atk_evs;
    std::atomic<uint64_t>& def_evs;
    std::atomic<uint64_t>& spa_evs;
    std::atomic<uint64_t>& spd_evs;
    std::atomic<uint64_t>& spe_evs;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> EvTrainer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

EvTrainer::EvTrainer()
    : LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , HP_EVS(
        "<b>HP EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    )
    , ATK_EVS(
        "<b>Attack EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    ) 
    , DEF_EVS(
        "<b>Defense EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    ) 
    , SPATK_EVS(
        "<b>Sp. Attack EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    ) 
    , SPDEF_EVS(
        "<b>Sp. Defense EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    )
    , SPEED_EVS(
        "<b>Speed EVs:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 255 // default, min, max
    )
    , PREVENT_EVOLUTION(
        "<b>Prevent " + Pokemon::STRING_POKEMON + " from evolving</b>",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , STOP_ON_MOVE_LEARN(
        "<b>Quit when a new move is learned</b><br>Stop this program when a new move is learned. If unchecked, new moves will not be learned.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , IGNORE_SHINIES(
        "<b>Ignore shinies</b><br>Do not stop the program when a wild shiny is encountered.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::UNLOCK_WHILE_RUNNING, 
        true // default
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    // PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP_EVS);
    PA_ADD_OPTION(ATK_EVS);
    PA_ADD_OPTION(DEF_EVS);
    PA_ADD_OPTION(SPATK_EVS);
    PA_ADD_OPTION(SPDEF_EVS);
    PA_ADD_OPTION(SPEED_EVS);
    PA_ADD_OPTION(PREVENT_EVOLUTION);
    PA_ADD_OPTION(STOP_ON_MOVE_LEARN);
    PA_ADD_OPTION(IGNORE_SHINIES);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void visit_pokecenter(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    enter_pokecenter(env.console, context);
    heal_at_pokecenter(env.console, context);
    leave_pokecenter(env.console, context);  
}

void travel_to_viridianforest(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    open_fly_map_from_overworld(env.console, context);
    fly_from_kanto_map(env.console, context, KantoFlyLocation::pewtercity);
    visit_pokecenter(env, context);
    env.log("Walking to Viridian Forest.");
    // down to the trees
    pbf_move_left_joystick(context, {0, -1}, 2350ms, 200ms);
    // right to the path
    pbf_move_left_joystick(context, {+1, 0}, 1150ms, 200ms);
    // down toward the gatehouse
    pbf_move_left_joystick(context, {0, -1}, 4150ms, 200ms);
    // left to line up with the door
    pbf_move_left_joystick(context, {-1, 0}, 950ms, 200ms);
    // down into the forest
    pbf_move_left_joystick(context, {0, -1}, 10000ms, 1000ms);
    context.wait_for_all_requests();
}

void travel_to_route22(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    open_fly_map_from_overworld(env.console, context);
    fly_from_kanto_map(env.console, context, KantoFlyLocation::viridiancity);
    visit_pokecenter(env, context);
    context.wait_for_all_requests();
    env.log("Walking to Route 22.");
    // left a few steps
    pbf_move_left_joystick(context, {-1, 0}, 900ms, 200ms);
    // up to the bush
    pbf_move_left_joystick(context, {0, +1}, 2300ms, 200ms);
    // left to the trees
    pbf_move_left_joystick(context, {-1, 0}, 7800ms, 200ms);
    // down and over the ledge
    pbf_move_left_joystick(context, {0, -1}, 3000ms, 200ms);
    // left a couple of steps
    pbf_move_left_joystick(context, {-1, 0}, 600ms, 200ms);
    // up to into the grass
    pbf_move_left_joystick(context, {0, +1}, 1500ms, 1000ms);
    context.wait_for_all_requests();
}

void travel_to_rock_tunnel(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    open_fly_map_from_overworld(env.console, context);
    fly_from_kanto_map(env.console, context, KantoFlyLocation::route10);
    visit_pokecenter(env, context);
    env.log("Walking to Rock Tunnel.");
    // down to the fence
    pbf_move_left_joystick(context, {0, -1}, 2367ms, 200ms);
    // left to the wall
    pbf_move_left_joystick(context, {-1, 0}, 3000ms, 200ms);
    // up to the wall
    pbf_move_left_joystick(context, {0, +1}, 2650ms, 200ms);
    // right to the entrance
    pbf_move_left_joystick(context, {+1, 0}, 1500ms, 200ms);
    // up into the entrance
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 4000ms);
    // down a few steps into the cave
    pbf_move_left_joystick(context, {0, -1}, 500ms, 1000ms);
    context.wait_for_all_requests();
}

void travel_to_pokemontower(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    open_fly_map_from_overworld(env.console, context);
    fly_from_kanto_map(env.console, context, KantoFlyLocation::lavendertown);
    visit_pokecenter(env, context);
    env.log("Walking to Pokemon Tower.");
    // PokeCenter to tower
    pbf_move_left_joystick(context, {0, -1}, 400ms, 200ms);
    pbf_move_left_joystick(context, {+1, 0}, 3200ms, 200ms);
    pbf_move_left_joystick(context, {0, +1}, 1550ms, 2400ms);
    // 1st floor to 2nd floor
    pbf_move_left_joystick(context, {0, +1}, 2350ms, 200ms);
    pbf_move_left_joystick(context, {+1, 0}, 3100ms, 1200ms);
    // 2nd floor to 3rd floor
    pbf_move_left_joystick(context, {0, +1}, 1000ms, 200ms);
    pbf_move_left_joystick(context, {-1, 0}, 3600ms, 200ms);
    pbf_move_left_joystick(context, {0, -1}, 1000ms, 200ms);
    pbf_move_left_joystick(context, {-1, 0}, 700ms, 1500ms);
    // take a few steps away from the stairs
    pbf_move_left_joystick(context, {0, +1}, 700ms, 1000ms);
    context.wait_for_all_requests();
}

void travel_to_surf_spot(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    int errors = 0;
    while (true){
        if (errors > 3){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to surf 3 times in a row.",
                env.console
            );
        }

        open_fly_map_from_overworld(env.console, context);
        fly_from_kanto_map(env.console, context, KantoFlyLocation::vermilioncity);
        visit_pokecenter(env, context);
        env.log("Walking to the shore and Surfing.");

        WhiteDialogWatcher surf_dialog(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                // walk up to counter and initiate dialog
                ssf_press_left_joystick(context, {-1, 0}, 0ms, 10000ms);
                ssf_mash1_button(context, BUTTON_A, 10000ms);
            },
            { surf_dialog }
        );
        if (ret < 0){
            env.log("Failed to detect surf dialog");
            errors++;
            pbf_mash_button(context, BUTTON_B, 1000ms);
            context.wait_for_all_requests();
            continue;
        }
        pbf_mash_button(context, BUTTON_A, 2000ms);
        // move away from the shore a little bit
        pbf_move_left_joystick(context, {-1, 0}, 3000ms, 200ms);
        context.wait_for_all_requests();
        return;
    }
}

void travel_to_route1(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    open_fly_map_from_overworld(env.console, context);
    fly_from_kanto_map(env.console, context, KantoFlyLocation::viridiancity);
    visit_pokecenter(env, context);
    context.wait_for_all_requests();
    env.log("Walking to Route 1.");
    // left a couple of steps
    pbf_move_left_joystick(context, {-1, 0}, 800ms, 100ms);
    // down to the tall grass
    pbf_move_left_joystick(context, {0, -1}, 5200ms, 100ms);
    context.wait_for_all_requests();
}

void use_dig(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    uint16_t errors = 0;
    while (true){
        if (errors > 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to use Dig 5 times in a row.",
                env.console
            );
        }

        open_party_menu_from_overworld(env.console, context);
        // navigate to 2nd to last party slot
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
        pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);

        PartySelectionWatcher teleporter_selected(COLOR_RED);

        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
            },
            { teleporter_selected }
        );

        if (ret < 0){
            env.log("Failed to select Dig user.");
            errors++;
            pbf_mash_button(context, BUTTON_B, 3000ms);
            continue;
        }
        
        // select Teleport (2nd option, but maybe HMs could change this)
        pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
        pbf_press_button(context, BUTTON_A, 200ms, 2800ms);

        BlackScreenWatcher dig_transition(COLOR_RED);

        context.wait_for_all_requests();
        ret = wait_until(
            env.console, context, 20000ms,
            {dig_transition}
        );

        if (ret < 0){
            env.log("Failed to use Dig");
            errors++;
            pbf_mash_button(context, BUTTON_B, 4000ms);
            continue;
        }

        pbf_wait(context, 3000ms);
        context.wait_for_all_requests();
        env.log("Used Dig.");
        return;
    }
}

int grass_spin(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool leftright){
    // "walk" without moving by tapping the joystick to change directions
    // alternate between left/right and up/down to ensure there is always a direction change

    BlackScreenWatcher battle_entered(COLOR_RED);

    context.wait_for_all_requests();
    env.log("Starting grass spin.");
    WallClock deadline = current_time() + 60s;

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [leftright, deadline](ProControllerContext& context) {
            while (current_time() < deadline){
                if (leftright){
                    pbf_move_left_joystick(context, {+1, 0}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {-1, 0}, 33ms, 150ms);
                }else{
                    pbf_move_left_joystick(context, {0, +1}, 33ms, 150ms);
                    pbf_move_left_joystick(context, {0, -1}, 33ms, 150ms);
                }
            }
        },
        { battle_entered }
    );
    
    if (ret < 0){
        return -1;
    }

    bool encounter_shiny = handle_encounter(env.console, context, true);
    return encounter_shiny ? 1 : 0;
}

} // namespace

std::string EvTrainer::get_encounter_species(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EvTrainingLocation& location){
    // make OCR more reliable by providing possible wild encounters for each location
    std::set<std::string> subset = {};
    switch (location){
    case EvTrainingLocation::viridianforest:
        subset = { "caterpie", "metapod", "weedle", "kakuna", "pikachu"};
        break;
    case EvTrainingLocation::route22:
        subset = { "rattata", "spearow", "mankey" };
        break;
    case EvTrainingLocation::rocktunnel:
        subset = { "geodude", "zubat", "mankey", "machop", "onix" };
        break;
    case EvTrainingLocation::pokemontower:
        subset = { "gastly", "haunter", "cubone" };
        break;
    case EvTrainingLocation::surfspot:
        subset = { "tentacool" };
        break;
    case EvTrainingLocation::route1:
        subset = { "pidgey", "rattata" };
    }

    context.wait_for_all_requests();

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading name...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter;
    reader.read_encounter(env.logger(), LANGUAGE, screen, subset, encounter);
    env.log("Name: " + encounter.name);

    return encounter.name;
}

EvTrainer::EffortValues EvTrainer::get_ev_yield(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string& species){
    std::set<std::string> subset;
    // It's probably a better idea to generate a more complete list of EVs to use as a resource
    // ... but this one is enough for the limited number of possible encounters in these few locations
    const std::map<std::string, EffortValues> ev_map = {
        {"caterpie",    {1,0,0,0,0,0}},
        {"metapod",     {0,0,2,0,0,0}},
        {"weedle",      {0,0,0,0,0,1}},
        {"kakuna",      {0,0,2,0,0,0}},
        {"pidgey",      {0,0,0,0,0,1}},
        {"rattata",     {0,0,0,0,0,1}},
        {"spearow",     {0,0,0,0,0,1}},
        {"pikachu",     {0,0,0,0,0,2}},
        {"zubat",       {0,0,0,0,0,1}},
        {"mankey",      {0,1,0,0,0,0}},
        {"machop",      {0,1,0,0,0,0}},
        {"tentacool",   {0,0,0,0,1,0}},
        {"geodude",     {0,0,1,0,0,0}},
        {"gastly",      {0,0,0,1,0,0}},
        {"haunter",     {0,0,0,2,0,0}},
        {"cubone",      {0,0,1,0,0,0}},
    };

    if (ev_map.find(species) == ev_map.end()){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Missing EV yield for " + species,
            env.console
        );
    }

    return ev_map.find(species)->second;
}

void EvTrainer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    EvTrainer_Descriptor::Stats& stats = env.current_stats<EvTrainer_Descriptor::Stats>();

    bool shiny_found = false;
    bool failed_encounter = false;

    bool spin_leftright = true;
    bool out_of_pp = false;
    
    EvTrainingLocation current_location = EvTrainingLocation::viridianforest;
    bool finished_stat = false;

    while (!shiny_found){
        try{
            if (stats.encounters == 0 || failed_encounter || finished_stat || out_of_pp){
                // use dig to get out of Pokemon Tower or Rock Tunnel
                if (current_location == EvTrainingLocation::pokemontower  || current_location == EvTrainingLocation::rocktunnel){
                    use_dig(env, context);
                }

                if (stats.hp_evs < HP_EVS){
                    current_location = EvTrainingLocation::viridianforest;
                }else if (stats.atk_evs < ATK_EVS){
                    current_location = EvTrainingLocation::route22;
                }else if (stats.def_evs < DEF_EVS){
                    current_location = EvTrainingLocation::rocktunnel;
                }else if (stats.spa_evs < SPATK_EVS){
                    current_location = EvTrainingLocation::pokemontower;
                }else if (stats.spd_evs < SPDEF_EVS){
                    current_location = EvTrainingLocation::surfspot;
                }else if (stats.spe_evs < SPEED_EVS){
                    current_location = EvTrainingLocation::route1;
                }else{
                    break;
                }

                spin_leftright = true;

                switch (current_location){
                case EvTrainingLocation::viridianforest:
                    travel_to_viridianforest(env, context);
                    break;
                case EvTrainingLocation::route22:
                    travel_to_route22(env, context);
                    break;
                case EvTrainingLocation::rocktunnel:
                    travel_to_rock_tunnel(env, context);
                    break;
                case EvTrainingLocation::pokemontower:
                    travel_to_pokemontower(env, context);
                    break;
                case EvTrainingLocation::surfspot:
                    travel_to_surf_spot(env, context);
                    spin_leftright = false;
                    break;
                case EvTrainingLocation::route1:
                    travel_to_route1(env, context);
                    break;
                default:
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Invalid EV option.",
                        env.console
                    );
                }
                
                stats.healing_trips++;
                out_of_pp = false;
                failed_encounter = false;
                finished_stat = false;
            }
               

            uint16_t errors = 0;
            int ret = grass_spin(env, context, spin_leftright);
            shiny_found = (ret == 1);
            if (ret < 0){
                failed_encounter = true;
                env.log("Failed to trigger encounter: restarting travel");
                errors++;
                if (errors >= 5){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed 5 times to trigger a wild encounter within 60 seconds",
                        env.console
                    );
                }
                // exit a menu in case there is one open
                pbf_mash_button(context, BUTTON_B, 1000ms);
                continue;
            }else{
                spin_leftright = !spin_leftright;
                stats.encounters++;
            }

            if (shiny_found && !IGNORE_SHINIES){
                env.log("Shiny found!");
                stats.shinies++;
                VideoSnapshot screen = env.console.video().snapshot();
                send_program_notification(
                    env,
                    NOTIFICATION_SHINY,
                    COLOR_YELLOW,
                    "Shiny found!",
                    {}, "",
                    screen,
                    true
                );
                if (TAKE_VIDEO){
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                break;
            }
            shiny_found = false;

            std::string encounter_species = get_encounter_species(env, context, current_location);
            EffortValues ev_yield = get_ev_yield(env, context, encounter_species);
            if (   (ev_yield.hp + stats.hp_evs > HP_EVS) 
                || (ev_yield.attack + stats.atk_evs> ATK_EVS)
                || (ev_yield.defense + stats.def_evs > DEF_EVS)
                || (ev_yield.spatk + stats.spa_evs > SPATK_EVS)
                || (ev_yield.spdef + stats.spd_evs > SPDEF_EVS)
                || (ev_yield.speed + stats.spe_evs> SPEED_EVS)
            ){
                flee_battle(env.console, context);
            }else{
                int ret2 = spam_first_move(env.console, context);
                if (ret2 == 1) { // user fainted
                    stats.times_fainted++;
                    out_of_pp = true; // triggers a healing trip
                    //TODO: handle exiting the battle in case the player can't escape
                    pbf_mash_button(context, BUTTON_B, 5000ms);
                    context.wait_for_all_requests();
                } else if (ret2 == 2){ // battle fled (no EV gain)
                    // continue;
                } else if (ret2 == 3){
                    out_of_pp = true;
                } else if (ret2 == 0){ // opponent fainted
                    stats.hp_evs  += ev_yield.hp;
                    stats.atk_evs += ev_yield.attack;
                    stats.def_evs += ev_yield.defense;
                    stats.spa_evs += ev_yield.spatk;
                    stats.spd_evs += ev_yield.spdef;
                    stats.spe_evs += ev_yield.speed;
                    
                    switch (current_location){
                    case EvTrainingLocation::viridianforest:
                        finished_stat = stats.hp_evs >= HP_EVS;
                        break;
                    case EvTrainingLocation::route22:
                        finished_stat = stats.atk_evs >= ATK_EVS;
                        break;
                    case EvTrainingLocation::rocktunnel:
                        finished_stat = stats.def_evs >= DEF_EVS;
                        break;
                    case EvTrainingLocation::pokemontower:
                        finished_stat = stats.spa_evs >= SPATK_EVS;
                        break;
                    case EvTrainingLocation::surfspot:
                        finished_stat = stats.spd_evs >= SPDEF_EVS;
                        break;
                    case EvTrainingLocation::route1:
                        finished_stat = stats.spe_evs >= SPEED_EVS;
                        break;
                    default:
                        finished_stat = false;
                    }

                    bool move_learned = exit_wild_battle(env.console, context, !!STOP_ON_MOVE_LEARN, !!PREVENT_EVOLUTION);

                    if (move_learned && STOP_ON_MOVE_LEARN){
                        send_program_status_notification(
                            env, NOTIFICATION_STATUS_UPDATE,
                            "Stopping: move learned."
                        );
                        break;
                    }
                }

            }            

            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "EV Training."
            );
            env.update_stats();
            context.wait_for_all_requests();

        }catch (OperationFailedException&){
            stats.errors++;
            throw;
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
