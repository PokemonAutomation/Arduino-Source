/*  Read Box Info
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <sstream>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellOccupiedDetector.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxCellReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxGenderReader.h"
#include "VideoGames/PokemonFRLG/Inference/Box/PokemonFRLG_BoxSelectionModeDetector.h"
#include "PokemonFRLG_ReadBoxInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ReadBoxInfo_Descriptor::ReadBoxInfo_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ReadBoxInfo",
        Pokemon::STRING_POKEMON + " FRLG", "Read Box Info", "",
        "Start on the box screen. Read one frame and log the selection hand, "
        "cursor location, occupancy, species, level, and gender. "
        "Detection regions remain visible for 10 seconds.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

ReadBoxInfo::ReadBoxInfo()
    : LANGUAGE(
        "<b>Game Language:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING, true
    )
{
    PA_ADD_OPTION(LANGUAGE);
}

void ReadBoxInfo::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    BoxSelectionModeDetector selection_detector(COLOR_YELLOW);
    BoxCellOccupiedDetector occupied_detector(COLOR_GREEN);
    BoxCellReader cell_reader(COLOR_RED);
    BoxGenderReader gender_reader(COLOR_BLUE);
    VideoOverlaySet overlays(env.console.overlay());
    selection_detector.make_overlays(overlays);
    occupied_detector.make_overlays(overlays);
    cell_reader.make_overlays(overlays);
    gender_reader.make_overlays(overlays);

    env.log("Reading FRLG box inference from one frame...");
    const VideoSnapshot screen = env.console.video().snapshot();
    if (!screen){
        env.log("Box inference: no video frame available.", COLOR_RED);
        return;
    }

    const auto selection = selection_detector.detect(screen);
    const bool occupied = occupied_detector.detect(screen);
    const std::string species = cell_reader.read_name(env.logger(), LANGUAGE, screen);
    const int level = cell_reader.read_level(env.logger(), screen);
    const auto gender = gender_reader.read(screen);

    // Print the report after OCR diagnostics so the results stay together.
    std::ostringstream report;
    report << "========== FRLG Box Inference ==========\n"
           << "Selection hand\n"
           << "  Detected:  " << (selection ? "Yes" : "No") << '\n';
    if (selection){
        report << "  Mode:      " << (selection->mode == BoxSelectionMode::Normal ? "Normal (white)" : "Relocate (gold)") << '\n'
               << "  Hand:      " << (selection->closed ? "Closed" : "Open") << '\n'
               << "  Location:  ";
        switch (selection->location){
        case BoxSelectionLocation::Grid:
            report << "Grid - row " << selection->row + 1 << ", column " << selection->column + 1 << " (1-based)";
            break;
        case BoxSelectionLocation::BoxTile:
            report << "Box title";
            break;
        case BoxSelectionLocation::OpenPartyButton:
            report << "Party Pokemon button";
            break;
        case BoxSelectionLocation::CloseBoxButton:
            report << "Close Box button";
            break;
        }
        report << '\n';
    }else{
        report << "  Mode/hand/location: Unknown\n";
    }
    report << "Selected Pokemon panel\n"
           << "  Occupied:  " << (occupied ? "Yes" : "No") << '\n'
           << "  Species:   " << (species.empty() ? "(empty or unreadable)" : species) << '\n'
           << "  Level:     " << (level == -1 ? "(empty or unreadable: -1)" : std::to_string(level)) << '\n'
           << "  Gender:    " << Pokemon::gender_to_string(gender) << '\n'
           << "========================================";
    env.log(report.str(), COLOR_BLUE);
    scope.wait_for(std::chrono::seconds(10));
}

}
}
}
