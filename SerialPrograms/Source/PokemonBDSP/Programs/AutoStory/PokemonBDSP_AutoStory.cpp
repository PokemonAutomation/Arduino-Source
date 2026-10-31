

#include <algorithm>
#include <memory>

#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"

#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"

#include "StorySegments/PokemonBDSP_AutoStory_Segment_00.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_01.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_02.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_03.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_04.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_05.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_06.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_07.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_08.h"
#include "StorySegments/PokemonBDSP_AutoStory_Segment_09.h"

#include "PokemonBDSP_AutoStory.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonBDSP {

using namespace std::chrono_literals;


// ---------------------------------------------------------------------------
//  Segment list
// ---------------------------------------------------------------------------

static std::vector<std::unique_ptr<AutoStory_Segment>> make_segment_list(){
    std::vector<std::unique_ptr<AutoStory_Segment>> segment_list;
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_00>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_01>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_02>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_03>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_04>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_05>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_06>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_07>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_08>());
    segment_list.emplace_back(std::make_unique<AutoStory_Segment_09>());
    return segment_list;
}

static const std::vector<std::unique_ptr<AutoStory_Segment>>& ALL_SEGMENTS(){
    static std::vector<std::unique_ptr<AutoStory_Segment>> list = make_segment_list();
    return list;
}
static std::vector<std::unique_ptr<AutoStory_Checkpoint>> make_checkpoint_list(){
    std::vector<std::unique_ptr<AutoStory_Checkpoint>> checkpoint_list;

    //Segment 00
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0000>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0001>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0002>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0003>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0004>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0005>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0006>());

    //Segment 01
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0101>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0102>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0103>());

    //Segment 02
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0201>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0202>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0203>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0204>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0205>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_0206>());

    //Segment 03
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0301>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0302>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0303>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0304>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0305>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0306>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0307>());

    // Segment 04
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0401>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0402>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0403>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0404>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0405>());

    // Segment 05
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0501>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0502>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0503>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0504>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0505>());

    // Segment 06
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0601>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0602>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0603>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0604>());

    // Segment 07
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0701>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0702>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0703>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0704>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0705>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0706>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0707>());

    // Segment 08
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0801>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0802>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0803>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0804>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0805>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0806>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0807>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0808>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0809>());

    // Segment 09
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0901>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0902>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0903>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0904>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0905>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0906>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0907>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0908>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_0909>());


    //checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_021>());
    /*checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_023>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_024>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_025>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_026>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_027>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_028>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_029>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_030>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_031>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_032>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_033>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_034>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_035>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_036>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_037>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_038>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_039>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_040>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_041>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_042>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_043>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_044>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_045>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_046>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_047>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_048>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_049>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_050>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_051>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_052>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_053>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_054>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_055>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_056>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_057>());*/

    return checkpoint_list;
}

static const std::vector<std::unique_ptr<AutoStory_Checkpoint>>& ALL_CHECKPOINTS(){
    static std::vector<std::unique_ptr<AutoStory_Checkpoint>> list = make_checkpoint_list();
    return list;
}

// ---------------------------------------------------------------------------
//  Segment database
// ---------------------------------------------------------------------------

// Preserve Tutorial and Segments 01-03 numeric selections. Revised Segments
// 04-09 use explicit slugs, and obsolete selections require reselection.
// Keep Tutorial and Main Story separate for the manual HOME transfer.
static constexpr size_t TUTORIAL_CHECKPOINT_COUNT = 7;
// Keep existing Main Story option IDs stable after splitting the tutorial.
static constexpr size_t LEGACY_MAIN_CHECKPOINT_START = 6;
static const StringSelectDatabase& section_database(bool checkpoints, bool tutorial){
    static const StringSelectDatabase tutorial_segments = [] {
        StringSelectDatabase db;
        db.add_entry(StringSelectEntry("0", ALL_SEGMENTS()[0]->name()));
        return db;
    }();
    static const StringSelectDatabase main_segments = [] {
        StringSelectDatabase db;
        for (size_t i = 1; i < ALL_SEGMENTS().size(); i++){
            db.add_entry(StringSelectEntry(std::to_string(i), ALL_SEGMENTS()[i]->name()));
        }
        return db;
    }();
    static const StringSelectDatabase tutorial_checkpoints = [] {
        StringSelectDatabase db;
        for (size_t i = 0; i < TUTORIAL_CHECKPOINT_COUNT; i++){
            const std::string name = ALL_CHECKPOINTS()[i]->name();
            db.add_entry(StringSelectEntry(name.substr(0, 5), name));
        }
        return db;
    }();
    static const StringSelectDatabase main_checkpoints = [] {
        StringSelectDatabase db;
        size_t legacy_index = LEGACY_MAIN_CHECKPOINT_START;
        for (size_t i = TUTORIAL_CHECKPOINT_COUNT; i < ALL_CHECKPOINTS().size(); i++){
            const std::string name = ALL_CHECKPOINTS()[i]->name();
            if (name.compare(0, 3, "04.") >= 0 && name.compare(0, 3, "09.") <= 0){
                db.add_entry(StringSelectEntry(name.substr(0, 5), name));
            }else{
                db.add_entry(StringSelectEntry(std::to_string(legacy_index++), name));
            }
        }
        db.add_entry(StringSelectEntry("checkpoint-reselect", "Select a revised checkpoint (old selection)"));
        return db;
    }();
    return checkpoints ? (tutorial ? tutorial_checkpoints : main_checkpoints)
        : (tutorial ? tutorial_segments : main_segments);
}

// ---------------------------------------------------------------------------
//  Descriptor
// ---------------------------------------------------------------------------

BDSPAutoStory_Descriptor::BDSPAutoStory_Descriptor()
    : SingleSwitchProgramDescriptor("PokemonBDSP:AutoStory",
        "Pokemon BDSP",
        "Auto Story",
        "Programs/PokemonBDSP/AutoStory.html",
        "Automatically completes BDSP story segments.",
        PanelQualityStanding::THIRD_PARTY_UNVETTED,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {"Tesseract"}){
}


std::unique_ptr<StatsTracker> BDSPAutoStory_Descriptor::make_stats() const {
    return std::make_unique<AutoStoryStats>();
}


// ---------------------------------------------------------------------------
//  Program Instance
// ---------------------------------------------------------------------------

BDSPAutoStory::~BDSPAutoStory(){
    OUTSIDE_HELP_POKEMON.remove_listener(*this);
    HELP_MODE.remove_listener(*this);
    STORY_SECTION.remove_listener(*this);
    START_SEGMENT_MAINSTORY.remove_listener(*this);
    END_SEGMENT_MAINSTORY.remove_listener(*this);
    START_CHECKPOINT_MAINSTORY.remove_listener(*this);
    END_CHECKPOINT_MAINSTORY.remove_listener(*this);
    START_CHECKPOINT_TUTORIAL.remove_listener(*this);
    END_CHECKPOINT_TUTORIAL.remove_listener(*this);
    START_SEGMENT.remove_listener(*this);
    END_SEGMENT.remove_listener(*this);
    ENABLE_ADVANCED_MODE.remove_listener(*this);
}


// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

BDSPAutoStory::BDSPAutoStory()
    : SingleSwitchProgramInstance()
    // DIAGNOSTIC ONLY: Remove before pushing to production.
    , DIAGNOSTIC_FAKE_CHECKPOINT_SAVE(
        "<b>Diagnostic only: Fake checkpoint saves</b><br>"
        "Use the fake save helper instead of normal checkpoint saving. "
        "Use an existing in-game save at the selected starting checkpoint or segment. "
        "Any failure reloads that save and reruns the entire selected range from its start. "
        "Checkpoint saves are not verified in this mode. Remove before pushing to production.",
        LockMode::LOCK_WHILE_RUNNING,
        false)
    , STORY_SECTION("<b>Story Section:</b>",
        {{StorySection::TUTORIAL, "tutorial", "Tutorial"}, {StorySection::MAIN_STORY, "main-story", "Main Story"}},
        LockMode::LOCK_WHILE_RUNNING,
        StorySection::TUTORIAL)
    , MAINSTORY_NOTE("",
        "Return your maxed starter from Pokemon HOME before starting Main Story. "
        "Use the required moveset and the starting position shown below.")
    , ENABLE_ADVANCED_MODE("<b>Advanced mode:</b><br>"
        "Use checkpoints instead of segments.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false)
    , START_CHECKPOINT_TUTORIAL("<b>Start Checkpoint:</b>",
        section_database(true, true),
        LockMode::LOCK_WHILE_RUNNING,
        "00.00")
    , END_CHECKPOINT_TUTORIAL("<b>End Checkpoint:</b>",
        section_database(true, true),
        LockMode::UNLOCK_WHILE_RUNNING,
        "00.06")

    , START_DESCRIPTION_SEGMENT("<b>Start:</b><br>", "")
    , END_DESCRIPTION_SEGMENT("<b>End:</b><br>", "")

    , START_DESCRIPTION_CHECKPOINT("<b>Start:</b><br>", "")
    , END_DESCRIPTION_CHECKPOINT("<b>End:</b><br>", "")

    , START_CHECKPOINT_MAINSTORY("<b>Start Checkpoint (Main):</b>",
        section_database(true, false),
        LockMode::LOCK_WHILE_RUNNING,
        "6")

    , END_CHECKPOINT_MAINSTORY("<b>End Checkpoint (Main):</b>",
        section_database(true, false),
        LockMode::UNLOCK_WHILE_RUNNING,
        section_database(true, false)[ALL_CHECKPOINTS().size() - TUTORIAL_CHECKPOINT_COUNT - 1].slug)

    , START_SEGMENT("<b>Start Segment:</b>", section_database(false, true), LockMode::LOCK_WHILE_RUNNING, "0")

    , END_SEGMENT("<b>End Segment:</b>", section_database(false, true), LockMode::UNLOCK_WHILE_RUNNING, "0")

    , START_SEGMENT_MAINSTORY("<b>Start Segment:</b>",
        section_database(false, false),
        LockMode::LOCK_WHILE_RUNNING,
        "1")
    , END_SEGMENT_MAINSTORY("<b>End Segment:</b>",
        section_database(false, false),
        LockMode::LOCK_WHILE_RUNNING,
        std::to_string(ALL_SEGMENTS().size() - 1))
    , STARTERCHOICE("<b>Starter Pokemon:</b>",
        {
            {StarterChoice::TURTWIG, "turtwig", "Turtwig"},
            {StarterChoice::CHIMCHAR, "chimchar", "Chimchar"},
            {StarterChoice::PIPLUP, "piplup", "Piplup"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StarterChoice::CHIMCHAR)

    , STARTER_SHINY("<b>Shiny Starter:</b>", LockMode::LOCK_WHILE_RUNNING, false)

    , CATCH_LEGENDARY("<b>Catch Box Legendary:</b>", LockMode::LOCK_WHILE_RUNNING, false)

    , HELP_MODE("<b>Help Mode:</b>",
        {
            {HelpMode::NO_OUTSIDE_HELP, "no_outside_help", "No Outside Help"},
            {HelpMode::OUTSIDE_HELP, "outside_help", "Outside Help"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        HelpMode::NO_OUTSIDE_HELP)

    , OUTSIDE_HELP_POKEMON("<b>Pokemon:</b>",
        {
            {OutsideHelpPokemon::TORTERRA, "torterra", "Torterra"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        OutsideHelpPokemon::TORTERRA)

    , OUTSIDE_HELP_ATTACK_1("<b>Attack 1:</b>",
        {
            {TorterraMove::EARTHQUAKE, "earthquake", "Earthquake"},
            {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
            {TorterraMove::RAZOR_LEAF, "razor_leaf", "Razor Leaf"},
            {TorterraMove::CRUNCH, "crunch", "Crunch"},
            {TorterraMove::GIGA_DRAIN, "giga_drain", "Giga Drain"},
            {TorterraMove::IRON_TAIL, "iron_tail", "Iron Tail"},
            {TorterraMove::ROCK_TOMB, "rock_tomb", "Rock Tomb"},
            {TorterraMove::FACADE, "facade", "Facade"},
            {TorterraMove::STONE_EDGE, "stone_edge", "Stone Edge"},
            {TorterraMove::ROCK_SLIDE, "rock_slide", "Rock Slide"},
            {TorterraMove::BULLDOZE, "bulldoze", "Bulldoze"},
            {TorterraMove::STRENGTH, "strength", "Strength"},
            {TorterraMove::ROCK_SMASH, "rock_smash", "Rock Smash"},
            {TorterraMove::ROCK_CLIMB, "rock_climb", "Rock Climb"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TorterraMove::EARTHQUAKE)

    , OUTSIDE_HELP_ATTACK_2("<b>Attack 2:</b>",
        {
            {TorterraMove::EARTHQUAKE, "earthquake", "Earthquake"},
            {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
            {TorterraMove::RAZOR_LEAF, "razor_leaf", "Razor Leaf"},
            {TorterraMove::CRUNCH, "crunch", "Crunch"},
            {TorterraMove::GIGA_DRAIN, "giga_drain", "Giga Drain"},
            {TorterraMove::IRON_TAIL, "iron_tail", "Iron Tail"},
            {TorterraMove::ROCK_TOMB, "rock_tomb", "Rock Tomb"},
            {TorterraMove::FACADE, "facade", "Facade"},
            {TorterraMove::STONE_EDGE, "stone_edge", "Stone Edge"},
            {TorterraMove::ROCK_SLIDE, "rock_slide", "Rock Slide"},
            {TorterraMove::BULLDOZE, "bulldoze", "Bulldoze"},
            {TorterraMove::STRENGTH, "strength", "Strength"},
            {TorterraMove::ROCK_SMASH, "rock_smash", "Rock Smash"},
            {TorterraMove::ROCK_CLIMB, "rock_climb", "Rock Climb"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TorterraMove::ROCK_SLIDE)

    , OUTSIDE_HELP_ATTACK_3("<b>Attack 3:</b>",
        {
            {TorterraMove::EARTHQUAKE, "earthquake", "Earthquake"},
            {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
            {TorterraMove::RAZOR_LEAF, "razor_leaf", "Razor Leaf"},
            {TorterraMove::CRUNCH, "crunch", "Crunch"},
            {TorterraMove::GIGA_DRAIN, "giga_drain", "Giga Drain"},
            {TorterraMove::IRON_TAIL, "iron_tail", "Iron Tail"},
            {TorterraMove::ROCK_TOMB, "rock_tomb", "Rock Tomb"},
            {TorterraMove::FACADE, "facade", "Facade"},
            {TorterraMove::STONE_EDGE, "stone_edge", "Stone Edge"},
            {TorterraMove::ROCK_SLIDE, "rock_slide", "Rock Slide"},
            {TorterraMove::BULLDOZE, "bulldoze", "Bulldoze"},
            {TorterraMove::STRENGTH, "strength", "Strength"},
            {TorterraMove::ROCK_SMASH, "rock_smash", "Rock Smash"},
            {TorterraMove::ROCK_CLIMB, "rock_climb", "Rock Climb"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TorterraMove::WOOD_HAMMER)

    , OUTSIDE_HELP_ATTACK_4("<b>Attack 4:</b>",
        {
            {TorterraMove::EARTHQUAKE, "earthquake", "Earthquake"},
            {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
            {TorterraMove::RAZOR_LEAF, "razor_leaf", "Razor Leaf"},
            {TorterraMove::CRUNCH, "crunch", "Crunch"},
            {TorterraMove::GIGA_DRAIN, "giga_drain", "Giga Drain"},
            {TorterraMove::IRON_TAIL, "iron_tail", "Iron Tail"},
            {TorterraMove::ROCK_TOMB, "rock_tomb", "Rock Tomb"},
            {TorterraMove::FACADE, "facade", "Facade"},
            {TorterraMove::STONE_EDGE, "stone_edge", "Stone Edge"},
            {TorterraMove::ROCK_SLIDE, "rock_slide", "Rock Slide"},
            {TorterraMove::BULLDOZE, "bulldoze", "Bulldoze"},
            {TorterraMove::STRENGTH, "strength", "Strength"},
            {TorterraMove::ROCK_SMASH, "rock_smash", "Rock Smash"},
            {TorterraMove::ROCK_CLIMB, "rock_climb", "Rock Climb"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        TorterraMove::CRUNCH)

    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(30))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })

{
    // -----------------------------------------------------------------------
    //  Options
    // -----------------------------------------------------------------------

    PA_ADD_OPTION(DIAGNOSTIC_FAKE_CHECKPOINT_SAVE);
    PA_ADD_OPTION(STORY_SECTION);
    PA_ADD_OPTION(MAINSTORY_NOTE);
    PA_ADD_OPTION(ENABLE_ADVANCED_MODE);

    // Segment UI
    PA_ADD_OPTION(START_SEGMENT);
    PA_ADD_OPTION(START_SEGMENT_MAINSTORY);
    PA_ADD_OPTION(START_DESCRIPTION_SEGMENT);

    PA_ADD_OPTION(END_SEGMENT);
    PA_ADD_OPTION(END_SEGMENT_MAINSTORY);
    PA_ADD_OPTION(END_DESCRIPTION_SEGMENT);

    // Checkpoint UI
    PA_ADD_OPTION(START_CHECKPOINT_TUTORIAL);
    PA_ADD_OPTION(START_CHECKPOINT_MAINSTORY);
    PA_ADD_OPTION(START_DESCRIPTION_CHECKPOINT);

    PA_ADD_OPTION(END_CHECKPOINT_TUTORIAL);
    PA_ADD_OPTION(END_CHECKPOINT_MAINSTORY);
    PA_ADD_OPTION(END_DESCRIPTION_CHECKPOINT);


    PA_ADD_OPTION(STARTERCHOICE);
    PA_ADD_OPTION(STARTER_SHINY);
    PA_ADD_OPTION(CATCH_LEGENDARY);

    PA_ADD_OPTION(HELP_MODE);
    PA_ADD_OPTION(OUTSIDE_HELP_POKEMON);

    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_1);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_2);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_3);
    PA_ADD_OPTION(OUTSIDE_HELP_ATTACK_4);

    PA_ADD_OPTION(NOTIFICATIONS);

    HELP_MODE.add_listener(*this);
    OUTSIDE_HELP_POKEMON.add_listener(*this);
    START_CHECKPOINT_TUTORIAL.add_listener(*this);
    END_CHECKPOINT_TUTORIAL.add_listener(*this);
    ENABLE_ADVANCED_MODE.add_listener(*this);
    START_SEGMENT.add_listener(*this);
    END_SEGMENT.add_listener(*this);
    STORY_SECTION.add_listener(*this);
    START_SEGMENT_MAINSTORY.add_listener(*this);
    END_SEGMENT_MAINSTORY.add_listener(*this);
    START_CHECKPOINT_MAINSTORY.add_listener(*this);
    END_CHECKPOINT_MAINSTORY.add_listener(*this);
    on_config_value_changed(nullptr);
}
std::string BDSPAutoStory::check_validity() const {
    const std::string error = SingleSwitchProgramInstance::check_validity();
    if (!error.empty()){
        return error;
    }
    if (ENABLE_ADVANCED_MODE && STORY_SECTION == StorySection::MAIN_STORY &&
        (START_CHECKPOINT_MAINSTORY.slug() == "checkpoint-reselect" ||
            END_CHECKPOINT_MAINSTORY.slug() == "checkpoint-reselect")){
        return "Checkpoints in Segments 04 through 09 have been consolidated. Select the revised start and end "
            "checkpoints that match your in-game save before starting.";
    }
    return {};
}

void BDSPAutoStory::load_json(const JsonValue& json){
    SingleSwitchProgramInstance::load_json(json);
    const JsonObject* object = json.to_object();
    if (!object){
        return;
    }
    const char* keys[] = {"START_CHECKPOINT_MAINSTORY", "END_CHECKPOINT_MAINSTORY"};
    StringSelectOption* selections[] = {&START_CHECKPOINT_MAINSTORY, &END_CHECKPOINT_MAINSTORY};
    for (size_t selection = 0; selection < 2; selection++){
        const std::string* slug = object->get_string(keys[selection]);
        if (!slug){
            continue;
        }
        if (*slug == "07.17" || *slug == "06-reselect" || *slug == "07-reselect"){
            selections[selection]->set_by_slug("checkpoint-reselect");
            continue;
        }
        for (size_t old_index = 22; old_index <= 125; old_index++){
            if (*slug == std::to_string(old_index)){
                selections[selection]->set_by_slug("checkpoint-reselect");
                break;
            }
        }
    }
    const char* tutorial_keys[] = {"START_CHECKPOINT_TUTORIAL", "END_CHECKPOINT_TUTORIAL"};
    StringSelectOption* tutorial_selections[] = {&START_CHECKPOINT_TUTORIAL, &END_CHECKPOINT_TUTORIAL};
    for (size_t selection = 0; selection < 2; selection++){
        const std::string* slug = object->get_string(tutorial_keys[selection]);
        if (!slug){
            continue;
        }
        for (size_t old_index = 0; old_index <= 5; old_index++){
            if (*slug == std::to_string(old_index)){
                size_t revised_index = old_index;
                if (old_index == 5){
                    revised_index = 6;
                }else if (old_index == 4 && selection == 1){
                    // Old 00.04 included both the catch and post-catch route.
                    revised_index = 5;
                }
                tutorial_selections[selection]->set_by_slug("00.0" + std::to_string(revised_index));
                break;
            }
        }
    }
    // Preserve the intent of a saved post-catch diagnostic configuration.
    const std::string* old_tutorial_start = object->get_string("START_CHECKPOINT_TUTORIAL");
    if (object->get_boolean_default("DIAGNOSTIC_POST_CATCH_ONLY") && old_tutorial_start && *old_tutorial_start == "4"){
        START_CHECKPOINT_TUTORIAL.set_by_slug("00.05");
        END_CHECKPOINT_TUTORIAL.set_by_slug("00.06");
    }
    on_config_value_changed(nullptr);
}

void BDSPAutoStory::update_ui_state(){
    const bool advanced = ENABLE_ADVANCED_MODE;
    const bool tutorial = STORY_SECTION == StorySection::TUTORIAL;
    auto visible = [](ConfigOption& option, bool show){
        option.set_visibility(show ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    };
    visible(START_SEGMENT, !advanced && tutorial);
    visible(END_SEGMENT, !advanced && tutorial);
    visible(START_SEGMENT_MAINSTORY, !advanced && !tutorial);
    visible(END_SEGMENT_MAINSTORY, !advanced && !tutorial);
    visible(START_CHECKPOINT_TUTORIAL, advanced && tutorial);
    visible(END_CHECKPOINT_TUTORIAL, advanced && tutorial);
    visible(START_CHECKPOINT_MAINSTORY, advanced && !tutorial);
    visible(END_CHECKPOINT_MAINSTORY, advanced && !tutorial);
    visible(START_DESCRIPTION_SEGMENT, !advanced);
    visible(END_DESCRIPTION_SEGMENT, !advanced);
    visible(START_DESCRIPTION_CHECKPOINT, advanced);
    visible(END_DESCRIPTION_CHECKPOINT, advanced);
    visible(MAINSTORY_NOTE, !tutorial);

    bool show_help = ((HelpMode)HELP_MODE == HelpMode::OUTSIDE_HELP);

    OUTSIDE_HELP_POKEMON.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);

    OUTSIDE_HELP_ATTACK_1.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_2.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_3.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_4.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
}
void BDSPAutoStory::on_config_value_changed(void*){
    const bool tutorial = STORY_SECTION == StorySection::TUTORIAL;
    if (ENABLE_ADVANCED_MODE){
        const auto& checkpoints = ALL_CHECKPOINTS();

        size_t start = tutorial ? START_CHECKPOINT_TUTORIAL.index()
            : START_CHECKPOINT_MAINSTORY.index() + TUTORIAL_CHECKPOINT_COUNT;
        size_t end = tutorial ? END_CHECKPOINT_TUTORIAL.index()
            : END_CHECKPOINT_MAINSTORY.index() + TUTORIAL_CHECKPOINT_COUNT;

        if (start < checkpoints.size()){
            START_DESCRIPTION_CHECKPOINT.set_text("Start: " + checkpoints[start]->start_text());
        }else{
            START_DESCRIPTION_CHECKPOINT.set_text("Select a revised checkpoint matching your in-game save.");
        }
        if (end < checkpoints.size()){
            END_DESCRIPTION_CHECKPOINT.set_text("End: " + checkpoints[end]->end_text());
        }else{
            END_DESCRIPTION_CHECKPOINT.set_text("Select a revised ending checkpoint.");
        }

    }else{
        const auto& segments = ALL_SEGMENTS();

        size_t start = std::stoul((tutorial ? START_SEGMENT : START_SEGMENT_MAINSTORY).slug());
        size_t end = std::stoul((tutorial ? END_SEGMENT : END_SEGMENT_MAINSTORY).slug());

        if (start < segments.size()){
            START_DESCRIPTION_SEGMENT.set_text("Start: " + segments[start]->start_text());
        }
        if (end < segments.size()){
            END_DESCRIPTION_SEGMENT.set_text("End: " + segments[end]->end_text());
        }
    }

    update_ui_state();
}
// ---------------------------------------------------------------------------
//  Program logic
// ---------------------------------------------------------------------------

void BDSPAutoStory::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const std::string error = check_validity();
    if (!error.empty()){
        throw FatalProgramException(ErrorReportMode::NO_ERROR_REPORT, error);
    }
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    AutoStoryOptions options{Language::English,
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE,
        (HelpMode)HELP_MODE,
        OutsideHelpConfig{(OutsideHelpPokemon)OUTSIDE_HELP_POKEMON,
            {
                (TorterraMove)OUTSIDE_HELP_ATTACK_1,
                (TorterraMove)OUTSIDE_HELP_ATTACK_2,
                (TorterraMove)OUTSIDE_HELP_ATTACK_3,
                (TorterraMove)OUTSIDE_HELP_ATTACK_4,
            }},
        DIAGNOSTIC_FAKE_CHECKPOINT_SAVE ? CheckpointSaveMode::FAKE : CheckpointSaveMode::REAL};

    stats.starter_choice = options.starter_choice;
    switch (options.starter_choice){
    case StarterChoice::TURTWIG:
        stats.move_pp_capacity = {15, 15, 10, 25};
        break;
    case StarterChoice::CHIMCHAR:
        stats.move_pp_capacity = {15, 15, 10, 30};
        break;
    case StarterChoice::PIPLUP:
        stats.move_pp_capacity = {20, 10, 15, 35};
        break;
    }
    // A direct main-story start requires full HP/PP, as described in the wiki.
    // After startup, only confirmed healing or a successful save reload refills it.
    stats.move_pp_remaining = stats.move_pp_capacity;

    const bool fake_saves = options.checkpoint_save_mode == CheckpointSaveMode::FAKE;
    if (fake_saves){
        options.checkpoint_retry_scope = CheckpointRetryScope::DEFER_TO_SELECTED_RANGE;
    }
    bool tutorial_completed = false;
    auto run_selected_range = [&](size_t attempt){
        if (attempt > 0){
            env.log("[Fake saves] Reloaded initial save; restarting the entire selected range from its start.");
        }


        const bool tutorial = STORY_SECTION == StorySection::TUTORIAL;
        const auto& segments = ALL_SEGMENTS();

        if (ENABLE_ADVANCED_MODE){
            size_t start = tutorial ? START_CHECKPOINT_TUTORIAL.index()
                : START_CHECKPOINT_MAINSTORY.index() + TUTORIAL_CHECKPOINT_COUNT;
            size_t end = tutorial ? END_CHECKPOINT_TUTORIAL.index()
                : END_CHECKPOINT_MAINSTORY.index() + TUTORIAL_CHECKPOINT_COUNT;

            env.log("Starting AutoStory from checkpoint " + std::to_string(start) + " to " + std::to_string(end));

            const auto& checkpoints = ALL_CHECKPOINTS();

            for (size_t i = start; i <= end && i < checkpoints.size(); i++){
                env.log("Running Checkpoint: " + std::to_string(i));
                checkpoints[i]->run_checkpoint(env, context, options, stats);
                if (tutorial && i + 1 == TUTORIAL_CHECKPOINT_COUNT){
                    tutorial_completed = true;
                }
            }

        }else{
            size_t start = std::stoul((tutorial ? START_SEGMENT : START_SEGMENT_MAINSTORY).slug());
            size_t end = std::stoul((tutorial ? END_SEGMENT : END_SEGMENT_MAINSTORY).slug());

            for (size_t i = start; i <= end && i < segments.size(); i++){
                env.log("Running Segment: " + std::to_string(i));
                segments[i]->run_segment(env, context, options, stats);
                if (tutorial && i == 0){
                    tutorial_completed = true;
                }
            }
        }
    };
    if (fake_saves){
        checkpoint_reattempt_loop(env,
            context,
            options.notif_status_update,
            stats,
            std::move(run_selected_range),
            true,
            CheckpointSaveMode::FAKE,
            CheckpointRetryScope::SELECTED_RANGE);
    }else{
        run_selected_range(0);
    }

    std::string completion_message;
    if (tutorial_completed){
        context.wait_for_all_requests();
        completion_message = "Tutorial complete. Deposit your starter in the Jubilife Pokemon Center PC, "
            "save and close BDSP, then transfer it through Pokemon HOME. Return your fully evolved "
            "level-100 starter to the first party slot with the wiki moveset and full HP and PP "
            "before starting Main Story.";
        MAINSTORY_NOTE.set_text(completion_message);
        MAINSTORY_NOTE.set_visibility(ConfigOptionState::ENABLED);
        env.log_to_ui(completion_message, COLOR_GREEN);
        env.console.log(completion_message, COLOR_GREEN);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, completion_message);
}


}
}
}