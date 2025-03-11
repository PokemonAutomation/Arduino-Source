/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SnapshotDumper_H
#define PokemonAutomation_NintendoSwitch_SnapshotDumper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SnapshotDumper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SnapshotDumper_Descriptor();
};


class SnapshotDumper : public SingleSwitchProgramInstance{
public:
    SnapshotDumper();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> PERIOD_MILLISECONDS;

    enum class Format{
        PNG,
        JPG,
    };
    EnumDropdownOption<Format> FORMAT;
};

// takes a snapshot of the screen and saves it to the given folder_name
void dump_snapshot(VideoStream& stream, std::string folder_name = "ScreenshotDumper");

}
}
#endif



