/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch_SnapshotDumper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SnapshotDumper_Descriptor::SnapshotDumper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:SnapshotDumper",
        "Nintendo Switch", "Snapshot Dumper",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/SnapshotDumper.md",
        "Periodically take screenshots.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}



SnapshotDumper::SnapshotDumper()
    : PERIOD_MILLISECONDS(
        "<b>Snapshot Period (milliseconds):</b><br>Take screenshot every this many milliseconds.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000
    )
    , FORMAT(
        "<b>Image Format:</b>",
        {
            {Format::PNG, "png", ".png"},
            {Format::JPG, "jpg", ".jpg"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        Format::JPG
    )
{
    PA_ADD_OPTION(PERIOD_MILLISECONDS);
    PA_ADD_OPTION(FORMAT);
}

void SnapshotDumper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::string folder_path = USER_FILE_PATH() + "ScreenshotDumper/";
    QDir().mkpath(folder_path.c_str());
    while (true){
        VideoSnapshot last = env.console.video().snapshot();
        std::string filename = folder_path + now_to_filestring();
        switch (FORMAT){
        case Format::PNG:
            filename += ".png";
            break;
        case Format::JPG:
            filename += ".jpg";
            break;
        }
        last->save(filename);
        context.wait_until(last.timestamp + std::chrono::milliseconds(PERIOD_MILLISECONDS));
    }
}

void dump_snapshot(VideoStream& stream, std::string folder_name){
    std::string folder_path = USER_FILE_PATH() + folder_name + "/";
    QDir().mkpath(folder_path.c_str());
    VideoSnapshot last = stream.video().snapshot();
    std::string filename = folder_path + now_to_filestring() + ".png";
    last->save(filename);
}


}
}

