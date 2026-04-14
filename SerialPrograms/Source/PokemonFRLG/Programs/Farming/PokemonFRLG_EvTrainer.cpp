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
    , MACHO_BRACE(
        "<b>Macho Brace:</b><br>Macho Brace doubles all earned EVs when held.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , POKERUS(
        "<b>" + Pokemon::STRING_POKERUS + ":</b><br>" + Pokemon::STRING_POKERUS + " doubles all earned EVs (stacks with Macho Brace)",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , PREVENT_EVOLUTION(
        "<b>Prevent " + Pokemon::STRING_POKEMON + " from evolving:</b>",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , STOP_ON_MOVE_LEARN(
        "<b>Quit when a new move is learned:</b><br>Stop this program when a new move is learned. If unchecked, new moves will not be learned.",
        LockMode::LOCK_WHILE_RUNNING, 
        false // default
    )
    , IGNORE_SHINIES(
        "<b>Ignore shinies:</b><br>Do not stop the program when a wild shiny is encountered.",
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
    PA_ADD_OPTION(MACHO_BRACE);
    PA_ADD_OPTION(POKERUS);
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

std::string get_ev_message(EvTrainer_Descriptor::Stats& stats){
    std::string message = "EVs earned: ";
    bool nonzero = false;
    if (stats.hp_evs){
        message += std::to_string(stats.hp_evs);
        message += " HP, ";
        nonzero = true;
    }
    if (stats.atk_evs){
        message += std::to_string(stats.atk_evs);
        message += " Attack, ";
        nonzero = true;
    }
    if (stats.def_evs){
        message += std::to_string(stats.def_evs);
        message += " Defense, ";
        nonzero = true;
    }
    if (stats.spa_evs){
        message += std::to_string(stats.spa_evs);
        message += " Sp. Attack, ";
        nonzero = true;
    }
    if (stats.spd_evs){
        message += std::to_string(stats.spd_evs);
        message += " Sp. Defense, ";
        nonzero = true;
    }
    if (stats.spe_evs){
        message += std::to_string(stats.spe_evs);
        message += " Speed, ";
        nonzero = true;
    }   

    if (!nonzero){
        message = "No EVs earned";
    }else{
        message.erase(message.size()-2);
    }

    return message;
}

} // namespace


bool EvTrainer::check_if_finished(EvTrainer_Descriptor::Stats& stats, uint8_t EV_MULTIPLIER
    , SimpleIntegerOption<uint64_t>& HP_EVS
    , SimpleIntegerOption<uint64_t>& ATK_EVS
    , SimpleIntegerOption<uint64_t>& DEF_EVS
    , SimpleIntegerOption<uint64_t>& SPATK_EVS
    , SimpleIntegerOption<uint64_t>& SPDEF_EVS
    , SimpleIntegerOption<uint64_t>& SPEED_EVS
){
    return ( stats.hp_evs + EV_MULTIPLIER - 1 > HP_EVS
        && stats.atk_evs + EV_MULTIPLIER - 1 > ATK_EVS
        && stats.def_evs + EV_MULTIPLIER - 1 > DEF_EVS
        && stats.spa_evs + EV_MULTIPLIER - 1 > SPATK_EVS
        && stats.spd_evs + EV_MULTIPLIER - 1 > SPDEF_EVS
        && stats.spe_evs + EV_MULTIPLIER - 1 > SPEED_EVS
    );
}

EvTrainer::EvTrainingLocation EvTrainer::get_next_location(SingleSwitchProgramEnvironment& env, EvTrainer_Descriptor::Stats& stats
    , SimpleIntegerOption<uint64_t>& HP_EVS
    , SimpleIntegerOption<uint64_t>& ATK_EVS
    , SimpleIntegerOption<uint64_t>& DEF_EVS
    , SimpleIntegerOption<uint64_t>& SPATK_EVS
    , SimpleIntegerOption<uint64_t>& SPDEF_EVS
    , SimpleIntegerOption<uint64_t>& SPEED_EVS
){
    if (stats.hp_evs < HP_EVS){
        return EvTrainingLocation::viridianforest;
    }else if (stats.atk_evs < ATK_EVS){
        return EvTrainingLocation::route22;
    }else if (stats.def_evs < DEF_EVS){
        return EvTrainingLocation::rocktunnel;
    }else if (stats.spa_evs < SPATK_EVS){
        return EvTrainingLocation::pokemontower;
    }else if (stats.spd_evs < SPDEF_EVS){
        return EvTrainingLocation::surfspot;
    }else if (stats.spe_evs < SPEED_EVS){
        return EvTrainingLocation::route1;
    }else{
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EvTrainer(): program failed to exit after earning all EVs",
            env.console
        );
    }
}

bool EvTrainer::travel_to_location(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EvTrainingLocation location){
    switch (location){
    case EvTrainingLocation::viridianforest:
        travel_to_viridianforest(env, context);
        return true;
    case EvTrainingLocation::route22:
        travel_to_route22(env, context);
        return true;
    case EvTrainingLocation::rocktunnel:
        travel_to_rock_tunnel(env, context);
        return true;
    case EvTrainingLocation::pokemontower:
        travel_to_pokemontower(env, context);
        return true;
    case EvTrainingLocation::surfspot:
        travel_to_surf_spot(env, context);
        return false;
    case EvTrainingLocation::route1:
        travel_to_route1(env, context);
        return true;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "EvTrainer: Invalid EV Training location.",
            env.console
        );
    }
}

std::string EvTrainer::get_encounter_species(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EvTrainingLocation location){
    const double MAX_SPECIES_LOG10P = -1.2; // relaxed from the default of -1.4 to prevent failure to get OCR matches for some species
    
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
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, subset, MAX_SPECIES_LOG10P);
    env.log("Name: " + encounter.name);

    return encounter.name;
}

EvTrainer::EffortValues EvTrainer::get_ev_yield(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::string& species, uint8_t EV_MULTIPLIER){
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
        {"onix",        {0,0,1,0,0,0}},
        {"cubone",      {0,0,1,0,0,0}},
    };

    if (species == ""){
        env.log("get_ev_yield(): failed to detect species");
        return {999, 999, 999, 999, 999, 999}; // this will always trigger running away
    }else if (ev_map.find(species) == ev_map.end()){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "get_ev_yield(): Missing EV yield for " + species,
            env.console
        );
    }

    EvTrainer::EffortValues evs = ev_map.find(species)->second;
    if (EV_MULTIPLIER > 1){
        evs = {
            evs.hp * EV_MULTIPLIER,
            evs.attack * EV_MULTIPLIER,
            evs.defense * EV_MULTIPLIER,
            evs.spatk * EV_MULTIPLIER,
            evs.spdef * EV_MULTIPLIER,
            evs.speed * EV_MULTIPLIER
        };
    }
    return evs;
}

EvTrainer::EvTrainerEncounterResult EvTrainer::trigger_wild_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EvTrainer_Descriptor::Stats& stats, bool spin_leftright){
    EvTrainerEncounterResult res = { false, false };

    int ret = grass_spin(env.console, context, spin_leftright);
    res.shiny_found = (ret == 1);
    if (ret < 0){
        res.failed_encounter = true;
        env.log("Failed to trigger encounter: restarting travel");
        // exit a menu in case there is one open
        pbf_mash_button(context, BUTTON_B, 1000ms);
    }else{
        stats.encounters++;
    }

    return res;
}


EvTrainer::EvTrainerBattleResult EvTrainer::handle_wild_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context
        , EvTrainer_Descriptor::Stats& stats, bool STOP_ON_MOVE_LEARN, bool PREVENT_EVOLUTION
        , uint8_t EV_MULTIPLIER, EvTrainingLocation location 
        , SimpleIntegerOption<uint64_t>& HP_EVS
        , SimpleIntegerOption<uint64_t>& ATK_EVS
        , SimpleIntegerOption<uint64_t>& DEF_EVS
        , SimpleIntegerOption<uint64_t>& SPATK_EVS
        , SimpleIntegerOption<uint64_t>& SPDEF_EVS
        , SimpleIntegerOption<uint64_t>& SPEED_EVS
){
    EvTrainerBattleResult res = { false, false, false };
    
    std::string encounter_species = get_encounter_species(env, context, location);
    EffortValues ev_yield = get_ev_yield(env, context, encounter_species, EV_MULTIPLIER);
    if (   (ev_yield.hp + stats.hp_evs > HP_EVS) 
        || (ev_yield.attack + stats.atk_evs> ATK_EVS)
        || (ev_yield.defense + stats.def_evs > DEF_EVS)
        || (ev_yield.spatk + stats.spa_evs > SPATK_EVS)
        || (ev_yield.spdef + stats.spd_evs > SPDEF_EVS)
        || (ev_yield.speed + stats.spe_evs> SPEED_EVS)
    ){
        flee_battle(env.console, context);
    }else{
        BattleResult ret = spam_first_move(env.console, context);
        if (ret == BattleResult::playerfainted) {
            stats.times_fainted++;
            res.should_heal = true;
            //TODO: handle exiting the battle in case the player can't escape (low speed, no Smoke Ball)
            pbf_mash_button(context, BUTTON_B, 5000ms);
            context.wait_for_all_requests();
        } else if (ret == BattleResult::unknown){ // battle fled (no EV gain)
            // continue;
        } else if (ret == BattleResult::outofpp){
            res.should_heal = true;
        } else if (ret == BattleResult::opponentfainted){
            stats.hp_evs  += ev_yield.hp;
            stats.atk_evs += ev_yield.attack;
            stats.def_evs += ev_yield.defense;
            stats.spa_evs += ev_yield.spatk;
            stats.spd_evs += ev_yield.spdef;
            stats.spe_evs += ev_yield.speed;
            
            switch (location){
            case EvTrainingLocation::viridianforest:
                res.finished_stat = stats.hp_evs + EV_MULTIPLIER - 1 >= HP_EVS; // avoid overshooting with Macho Brace / Pokerus
                break;
            case EvTrainingLocation::route22:
                res.finished_stat = stats.atk_evs + EV_MULTIPLIER - 1 >= ATK_EVS;
                break;
            case EvTrainingLocation::rocktunnel:
                res.finished_stat = stats.def_evs + EV_MULTIPLIER - 1 >= DEF_EVS;
                break;
            case EvTrainingLocation::pokemontower:
                res.finished_stat = stats.spa_evs + EV_MULTIPLIER - 1 >= SPATK_EVS;
                break;
            case EvTrainingLocation::surfspot:
                res.finished_stat = stats.spd_evs + EV_MULTIPLIER - 1 >= SPDEF_EVS;
                break;
            case EvTrainingLocation::route1:
                res.finished_stat = stats.spe_evs + EV_MULTIPLIER - 1 >= SPEED_EVS;
                break;
            default:
                res.finished_stat = false;
            }

            res.move_learned = exit_wild_battle(env.console, context, !!STOP_ON_MOVE_LEARN, !!PREVENT_EVOLUTION);
        }
    }  

    return res;
}

void EvTrainer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    EvTrainer_Descriptor::Stats& stats = env.current_stats<EvTrainer_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    uint8_t EV_MULTIPLIER = (MACHO_BRACE ? 2 : 1) * (POKERUS ? 2 : 1);

    bool failed_last_encounter = false;
    uint8_t failed_encounters = 0;

    bool spin_leftright = true;
    bool should_heal = false;

    EvTrainingLocation current_location = EvTrainingLocation::viridianforest;

    bool finished_stat = false;
    bool finished_all = check_if_finished(stats, EV_MULTIPLIER, HP_EVS, ATK_EVS, DEF_EVS, SPATK_EVS, SPDEF_EVS, SPEED_EVS);

    while (!finished_all){
        try{
            if (failed_encounters >= 5){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to trigger a wild encounter within 60 seconds 5 times in a row",
                    env.console
                );
            }

            // travel to the relevant Fly spot, heal at a PokeCenter, and walk to the grass
            if (stats.encounters == 0 || failed_last_encounter || finished_stat || should_heal){
                // use dig to get out of Pokemon Tower or Rock Tunnel
                if (current_location == EvTrainingLocation::pokemontower  || current_location == EvTrainingLocation::rocktunnel){
                    use_dig(env, context);
                }

                current_location = get_next_location(env, stats, HP_EVS, ATK_EVS, DEF_EVS, SPATK_EVS, SPDEF_EVS, SPEED_EVS);
                spin_leftright = travel_to_location(env, context, current_location);

                stats.healing_trips++;
                should_heal = false;
                failed_last_encounter = false;
                finished_stat = false;
            }

               
            // trigger a wild encounter
            EvTrainerEncounterResult encounter_res = trigger_wild_encounter(env, context, stats, spin_leftright);
            failed_last_encounter = encounter_res.failed_encounter;
            if (failed_last_encounter){
                failed_encounters++;
                continue;
            }else{
                spin_leftright = !spin_leftright;
                failed_encounters = 0;
            }


            // handle shiny encounters
            if (encounter_res.shiny_found){
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
                if (!IGNORE_SHINIES){
                    break;
                }
            }


            // handle the battle and avoid overshooting the target EVs
            EvTrainerBattleResult battle_res = handle_wild_battle(env, context, stats
                , !!STOP_ON_MOVE_LEARN, !!PREVENT_EVOLUTION , EV_MULTIPLIER, current_location
                , HP_EVS , ATK_EVS, DEF_EVS, SPATK_EVS, SPDEF_EVS, SPEED_EVS);

            should_heal = battle_res.should_heal;
            finished_stat = battle_res.finished_stat;
            finished_all = check_if_finished(stats, EV_MULTIPLIER, HP_EVS, ATK_EVS, DEF_EVS, SPATK_EVS, SPDEF_EVS, SPEED_EVS);

            if (battle_res.move_learned && STOP_ON_MOVE_LEARN){
                VideoSnapshot screen = env.console.video().snapshot();
                send_program_notification(
                    env, 
                    NOTIFICATION_STATUS_UPDATE,
                    COLOR_BLUE,
                    "Stopping: move learned.",
                    {}, "",
                    screen,
                    true
                );
                break;
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

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, get_ev_message(stats));
}

}
}
}
