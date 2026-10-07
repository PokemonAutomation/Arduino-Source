/*  Wild Pokemon Finder
 *
 *  Search grass encounters until a selected species is found.
 */

#include <set>
#include <string>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "VideoGames/PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"
#include "VideoGames/PokemonFRLG/Programs/RngManipulation/PokemonFRLG_EncountersDatabase.h"
#include "PokemonFRLG_WildPokemonFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{

std::set<std::string> grass_species_in_database(const EncountersDatabase& database){
    std::set<std::string> species;
    for (const auto& [location, slots] : database.get_throw("grass")){
        for (const AdvEncounterSlot& slot : slots){
            species.insert(slot.species);
        }
    }
    return species;
}

std::vector<std::string> all_grass_species(){
    std::set<std::string> species;
    for (const char* path : {
        "PokemonFRLG/EncounterSlotsFR.json",
        "PokemonFRLG/EncounterSlotsLG.json",
    }){
        EncountersDatabase database(path);
        const std::set<std::string> game_species = grass_species_in_database(database);
        species.insert(game_species.begin(), game_species.end());
    }
    return std::vector<std::string>(species.begin(), species.end());
}

void find_next_battle(ConsoleHandle& console, ProControllerContext& context){
    BlackScreenWatcher battle_entered(COLOR_RED);
    int result = 1;
    while (result != 0){
        result = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                ssf_press_left_joystick(context, { -1, 0 }, 0ms, 1000ms);
                ssf_mash1_button(context, BUTTON_B, 936ms);
                ssf_press_left_joystick(context, { +1, 0 }, 0ms, 1000ms);
                ssf_mash1_button(context, BUTTON_B, 936ms);
            },
            { battle_entered }
        );
    }

    BattleMenuWatcher battle_menu(COLOR_RED);
    WallClock deadline = current_time() + 60s;
    while (current_time() < deadline){
        result = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 5000ms);
            },
            { battle_menu }
        );
        if (result == 0){
            return;
        }
    }

    OperationFailedExceptionWithScreenshot::fire(
        ErrorReportMode::SEND_ERROR_REPORT,
        "WildPokemonFinder: Battle menu was not detected after entering an encounter.",
        console
    );
}

}

WildPokemonFinder_Descriptor::WildPokemonFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:WildPokemonFinder",
        Pokemon::STRING_POKEMON + " FRLG", "Wild Pokemon Finder",
        "Programs/PokemonFRLG/WildPokemonFinder.html",
        "Search grass encounters for a selected Pokemon. Start in grass where the target appears, and use a lead that can always flee.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct WildPokemonFinder_Descriptor::Stats : public StatsTracker{
    Stats()
        : encounters(m_stats["Encounters"])
        , unreadable(m_stats["Unreadable Encounters"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Unreadable Encounters", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& unreadable;
};

std::unique_ptr<StatsTracker> WildPokemonFinder_Descriptor::make_stats() const{
    return std::make_unique<Stats>();
}

WildPokemonFinder::WildPokemonFinder()
    : GAME_VERSION(
        "<b>Game Version:</b>",
        {
            {GameVersion::firered, "firered", "FireRed"},
            {GameVersion::leafgreen, "leafgreen", "LeafGreen"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        GameVersion::firered
    )
    , TARGET_POKEMON(
        "<b>Target Wild Pokemon:</b><br>Only Pokemon available in grass encounters are listed."
        " Choose a target available in the selected game version.",
        all_grass_species(),
        "pikachu"
    )
    , LANGUAGE(
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
    , NOTIFICATION_TARGET_FOUND(
        "Target Pokemon found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_TARGET_FOUND,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(GAME_VERSION);
    PA_ADD_OPTION(TARGET_POKEMON);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void WildPokemonFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WildPokemonFinder_Descriptor::Stats& stats =
        env.current_stats<WildPokemonFinder_Descriptor::Stats>();

    const bool firered = GAME_VERSION == GameVersion::firered;
    EncountersDatabase encounter_database(
        firered ? "PokemonFRLG/EncounterSlotsFR.json" : "PokemonFRLG/EncounterSlotsLG.json"
    );
    const std::set<std::string> available_species = grass_species_in_database(encounter_database);
    const std::string target = TARGET_POKEMON.slug();
    if (available_species.find(target) == available_species.end()){
        throw UserSetupError(
            env.console,
            "The selected Pokemon is not available in grass encounters in "
                + std::string(firered ? "FireRed." : "LeafGreen.")
        );
    }

    home_black_border_check(env.console, context);

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Searching for wild Pokemon: " + target);
    while (true){
        find_next_battle(env.console, context);

        VideoSnapshot screen = env.console.video().snapshot();
        PokemonFRLG_WildEncounter encounter = reader.read_encounter(
            env.logger(), LANGUAGE, screen, available_species
        );
        stats.encounters++;
        env.update_stats();

        if (encounter.name == target){
            const std::string display_name = TARGET_POKEMON.display_name();
            env.log("Target Pokemon found: " + display_name, COLOR_GREEN);
            send_program_notification(
                env,
                NOTIFICATION_TARGET_FOUND,
                COLOR_GREEN,
                "Target Pokemon found: " + display_name,
                {}, "",
                screen,
                true
            );
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
            return;
        }

        if (encounter.name.empty()){
            stats.unreadable++;
            env.log("Unable to read the wild Pokemon; fleeing and trying again.", COLOR_RED);
            env.update_stats();
        }else{
            env.log("Found " + encounter.name + "; target is " + target + ".");
        }

        flee_battle(env.console, context);
        context.wait_for_all_requests();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
}

}
}
}