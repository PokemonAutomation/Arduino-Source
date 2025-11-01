/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SnapshotDumper_H
#define PokemonAutomation_NintendoSwitch_SnapshotDumper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SnapshotDumper_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SnapshotDumper_Descriptor();
};

enum class Format{
    PNG,
    JPG,
};

class SnapshotDumper : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    ~SnapshotDumper();
    SnapshotDumper();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerOption<uint32_t> PERIOD_MILLISECONDS;
    BooleanCheckBoxOption CLICK_TO_SNAPSHOT;
    EnumDropdownOption<Format> FORMAT;
};

std::string to_format_string(Format format);

// takes a snapshot of the screen and saves it to the given folder_name
void dump_snapshot(VideoStream& stream, std::string folder_name = "ScreenshotDumper", std::string format = ".png");

}
}
#endif



