#include <algorithm>
#include <memory>

#include "PokemonBDSP_AutoStory.h"

#include "CommonFramework/Globals.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonTools/Async/InferenceRoutines.h"
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

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

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

    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_000>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_001>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_002>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_003>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_004>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_005>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_006>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_007>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_008>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_009>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_checkpoint_010>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_011>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_012>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_013>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_014>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_015>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_016>());
    // Segment 04 (050-066)
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_050>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_051>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_052>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_053>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_054>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_055>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_056>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_057>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_058>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_059>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_060>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_061>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_062>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_063>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_064>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_065>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_066>());
    // Segment 05 (067-082)
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_067>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_068>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_069>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_070>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_071>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_072>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_073>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_074>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_075>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_076>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_077>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_078>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_079>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_080>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_081>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_082>());
    // Segment 06 (083-092)
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_083>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_084>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_085>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_086>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_087>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_088>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_089>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_090>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_091>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_092>());
    // Segment 07 (093-110)
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_093>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_094>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_095>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_096>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_097>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_098>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_099>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_100>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_101>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_102>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_103>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_104>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_105>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_106>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_107>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_108>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_109>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_110>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_111>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_112>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_113>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_114>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_115>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_116>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_117>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_118>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_119>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_120>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_121>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_122>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_123>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_124>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_125>());



    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_021>());
    checkpoint_list.emplace_back(std::make_unique<AutoStory_Checkpoint_022>());
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

StringSelectDatabase make_segment_database(){
    StringSelectDatabase db;
    int index = 0;
    for (const auto& segment : ALL_SEGMENTS()){
        db.add_entry(StringSelectEntry(std::to_string(index), segment->name()));
        index++;
    }
    return db;
}
// ---------------------------------------------------------------------------
//  Checkpoint database (SV-style)
// ---------------------------------------------------------------------------
const StringSelectDatabase& SEGMENT_DATABASE(){
    static StringSelectDatabase db = make_segment_database();
    return db;
}
StringSelectDatabase make_checkpoint_database(){
    StringSelectDatabase db;
    int index = 0;
    for (const auto& checkpoint : ALL_CHECKPOINTS()){
        db.add_entry(StringSelectEntry(std::to_string(index), checkpoint->name()));
        index++;
    }
    return db;
}

const StringSelectDatabase& CHECKPOINT_DATABASE(){
    static StringSelectDatabase db = make_checkpoint_database();
    return db;
}

// ---------------------------------------------------------------------------
//  Descriptor
// ---------------------------------------------------------------------------

BDSPAutoStory_Descriptor::BDSPAutoStory_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonBDSP:AutoStory",
          "Pokemon BDSP",
          "Auto Story",
          "Programs/PokemonBDSP/AutoStory.html",
          "Automatically completes BDSP story segments.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED,
          AllowCommandsWhenRunning::DISABLE_COMMANDS
          )
{}


std::unique_ptr<SingleSwitchProgramInstance>
BDSPAutoStory_Descriptor::make_instance() const{
    return std::make_unique<BDSPAutoStory>();
}

std::unique_ptr<StatsTracker>
BDSPAutoStory_Descriptor::make_stats() const{
    return std::make_unique<AutoStoryStats>();
}


// ---------------------------------------------------------------------------
//  Program Instance
// ---------------------------------------------------------------------------

BDSPAutoStory::~BDSPAutoStory(){
    OUTSIDE_HELP_POKEMON.remove_listener(*this);
    HELP_MODE.remove_listener(*this);

}


// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

BDSPAutoStory::BDSPAutoStory()
    : SingleSwitchProgramInstance()
    , ENABLE_ADVANCED_MODE(
          "<b>Advanced mode:</b><br>"
          "Use checkpoints instead of segments.",
          LockMode::UNLOCK_WHILE_RUNNING,
          false
          )
    , START_CHECKPOINT_TUTORIAL(
          "<b>Start Checkpoint:</b>",
          CHECKPOINT_DATABASE(),
          LockMode::LOCK_WHILE_RUNNING,
          "0"
          )
    , END_CHECKPOINT_TUTORIAL(
          "<b>End Checkpoint:</b>",
          CHECKPOINT_DATABASE(),
          LockMode::UNLOCK_WHILE_RUNNING,
          "2"
          )

    , START_DESCRIPTION_SEGMENT("<b>Start:</b><br>", "")
    , END_DESCRIPTION_SEGMENT("<b>End:</b><br>", "")

    , START_DESCRIPTION_CHECKPOINT("<b>Start:</b><br>", "")
    , END_DESCRIPTION_CHECKPOINT("<b>End:</b><br>", "")

    , START_CHECKPOINT_MAINSTORY(
          "<b>Start Checkpoint (Main):</b>",
          CHECKPOINT_DATABASE(),
          LockMode::LOCK_WHILE_RUNNING,
          "0"
          )

    , END_CHECKPOINT_MAINSTORY(
          "<b>End Checkpoint (Main):</b>",
          CHECKPOINT_DATABASE(),
          LockMode::UNLOCK_WHILE_RUNNING,
          "2"
          )

    , START_SEGMENT(
        "<b>Start Segment:</b>",
        SEGMENT_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        "0"
    )

    , END_SEGMENT(
        "<b>End Segment:</b>",
        SEGMENT_DATABASE(),
        LockMode::UNLOCK_WHILE_RUNNING,
        std::to_string(ALL_SEGMENTS().size() - 1)
    )

    , STARTERCHOICE(
          "<b>Starter Pokemon:</b>",
          {
           {StarterChoice::TURTWIG,  "turtwig",  "Turtwig"},
           {StarterChoice::CHIMCHAR, "chimchar", "Chimchar"},
           {StarterChoice::PIPLUP,   "piplup",   "Piplup"},
           },
          LockMode::LOCK_WHILE_RUNNING,
          StarterChoice::CHIMCHAR
          )

    , STARTER_SHINY(
          "<b>Shiny Starter:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )

    , CATCH_LEGENDARY(
          "<b>Catch Box Legendary:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )

    , HELP_MODE(
          "<b>Help Mode:</b>",
          {
              {HelpMode::NO_OUTSIDE_HELP, "no_outside_help", "No Outside Help"},
              {HelpMode::OUTSIDE_HELP,    "outside_help",    "Outside Help"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          HelpMode::NO_OUTSIDE_HELP
          )

    , OUTSIDE_HELP_POKEMON(
          "<b>Pokemon:</b>",
          {
              {OutsideHelpPokemon::TORTERRA, "torterra", "Torterra"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          OutsideHelpPokemon::TORTERRA
          )

    , OUTSIDE_HELP_ATTACK_1(
          "<b>Attack 1:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::EARTHQUAKE
          )

    , OUTSIDE_HELP_ATTACK_2(
          "<b>Attack 2:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::ROCK_SLIDE
          )

    , OUTSIDE_HELP_ATTACK_3(
          "<b>Attack 3:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::WOOD_HAMMER
          )

    , OUTSIDE_HELP_ATTACK_4(
          "<b>Attack 4:</b>",
          {
              {TorterraMove::EARTHQUAKE,  "earthquake",  "Earthquake"},
              {TorterraMove::WOOD_HAMMER, "wood_hammer", "Wood Hammer"},
              {TorterraMove::RAZOR_LEAF,  "razor_leaf",  "Razor Leaf"},
              {TorterraMove::CRUNCH,      "crunch",      "Crunch"},
              {TorterraMove::GIGA_DRAIN,  "giga_drain",  "Giga Drain"},
              {TorterraMove::IRON_TAIL,   "iron_tail",   "Iron Tail"},
              {TorterraMove::ROCK_TOMB,   "rock_tomb",   "Rock Tomb"},
              {TorterraMove::FACADE,      "facade",      "Facade"},
              {TorterraMove::STONE_EDGE,  "stone_edge",  "Stone Edge"},
              {TorterraMove::ROCK_SLIDE,  "rock_slide",  "Rock Slide"},
              {TorterraMove::BULLDOZE,    "bulldoze",    "Bulldoze"},
              {TorterraMove::STRENGTH,    "strength",    "Strength"},
              {TorterraMove::ROCK_SMASH,  "rock_smash",  "Rock Smash"},
              {TorterraMove::ROCK_CLIMB,  "rock_climb",  "Rock Climb"},
          },
          LockMode::LOCK_WHILE_RUNNING,
          TorterraMove::CRUNCH
          )

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

    PA_ADD_OPTION(ENABLE_ADVANCED_MODE);

    // Segment UI
    PA_ADD_OPTION(START_SEGMENT);
    PA_ADD_OPTION(START_DESCRIPTION_SEGMENT);

    PA_ADD_OPTION(END_SEGMENT);
    PA_ADD_OPTION(END_DESCRIPTION_SEGMENT);

    // Checkpoint UI
    PA_ADD_OPTION(START_CHECKPOINT_TUTORIAL);
    PA_ADD_OPTION(START_DESCRIPTION_CHECKPOINT);

    PA_ADD_OPTION(END_CHECKPOINT_TUTORIAL);
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
    update_ui_state();
}
void BDSPAutoStory::update_ui_state(){
    bool advanced = ENABLE_ADVANCED_MODE;
    START_DESCRIPTION_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );
    END_DESCRIPTION_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    START_DESCRIPTION_CHECKPOINT.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
    END_DESCRIPTION_CHECKPOINT.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
    START_DESCRIPTION_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );
    END_DESCRIPTION_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    START_DESCRIPTION_CHECKPOINT.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
    END_DESCRIPTION_CHECKPOINT.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    bool show_help = ((HelpMode)HELP_MODE == HelpMode::OUTSIDE_HELP);

    OUTSIDE_HELP_POKEMON.set_visibility(
        show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );

    OUTSIDE_HELP_ATTACK_1.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_2.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_3.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
    OUTSIDE_HELP_ATTACK_4.set_visibility(show_help ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN);
}
void BDSPAutoStory::on_config_value_changed(void*){
    bool advanced = ENABLE_ADVANCED_MODE;

    // Segments
    START_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );
    END_SEGMENT.set_visibility(
        advanced ? ConfigOptionState::HIDDEN : ConfigOptionState::ENABLED
    );

    // Checkpoints (tutorial for now)
    START_CHECKPOINT_TUTORIAL.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
    END_CHECKPOINT_TUTORIAL.set_visibility(
        advanced ? ConfigOptionState::ENABLED : ConfigOptionState::HIDDEN
    );
    if (ENABLE_ADVANCED_MODE){
        const auto& checkpoints = ALL_CHECKPOINTS();

        size_t start = START_CHECKPOINT_TUTORIAL.index();
        size_t end   = END_CHECKPOINT_TUTORIAL.index();

        if (start < checkpoints.size()){
            START_DESCRIPTION_CHECKPOINT.set_text(
                "Start: " + checkpoints[start]->start_text()
            );
        }
        if (end < checkpoints.size()){
            END_DESCRIPTION_CHECKPOINT.set_text(
                "End: " + checkpoints[end]->end_text()
            );
        }

    }else{
        const auto& segments = ALL_SEGMENTS();

        size_t start = START_SEGMENT.index();
        size_t end   = END_SEGMENT.index();

        if (start < segments.size()){
            START_DESCRIPTION_SEGMENT.set_text(
                "Start: " + segments[start]->start_text()
            );
        }
        if (end < segments.size()){
            END_DESCRIPTION_SEGMENT.set_text(
                "End: " + segments[end]->end_text()
            );
        }
    }

    update_ui_state();
}
// ---------------------------------------------------------------------------
//  Program logic
// ---------------------------------------------------------------------------

void BDSPAutoStory::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    AutoStoryStats& stats = env.current_stats<AutoStoryStats>();

    AutoStoryOptions options{
        Language::English,
        STARTERCHOICE,
        NOTIFICATION_STATUS_UPDATE,
        (HelpMode)HELP_MODE,
        OutsideHelpConfig{
            (OutsideHelpPokemon)OUTSIDE_HELP_POKEMON,
            {
                (TorterraMove)OUTSIDE_HELP_ATTACK_1,
                (TorterraMove)OUTSIDE_HELP_ATTACK_2,
                (TorterraMove)OUTSIDE_HELP_ATTACK_3,
                (TorterraMove)OUTSIDE_HELP_ATTACK_4,
            }
        }
    };

    const auto& segments = ALL_SEGMENTS();



    if (ENABLE_ADVANCED_MODE){
        size_t start = START_CHECKPOINT_TUTORIAL.index();
        size_t end   = END_CHECKPOINT_TUTORIAL.index();

        env.log("Starting AutoStory from checkpoint " + std::to_string(start) +
                " to " + std::to_string(end));

        const auto& checkpoints = ALL_CHECKPOINTS();

        for (size_t i = start; i <= end && i < checkpoints.size(); i++){
            env.log("Running Checkpoint: " + std::to_string(i));
            checkpoints[i]->run_checkpoint(env, context, options, stats);
        }

    }else{
        size_t start = START_SEGMENT.index();
        size_t end   = END_SEGMENT.index();

        for (size_t i = start; i <= end && i < segments.size(); i++){
            env.log("Running Segment: " + std::to_string(i));
            segments[i]->run_segment(env, context, options, stats);
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}