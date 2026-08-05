/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SnapshotDumper_H
#define PokemonAutomation_NintendoSwitch_SnapshotDumper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "ControllerInput/ControllerInput.h"
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

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    SimpleIntegerOption<uint32_t> PERIOD_MILLISECONDS;
    enum class SnapshotMode{
        KEYPRESS,
        MOUSE_CLICK,
        PERIODIC,
    };
    EnumDropdownOption<SnapshotMode> SNAPSHOT_MODE;
    EnumDropdownOption<Format> FORMAT;
};


class SnapshotKeyTrigger : public ControllerInputListener{
public:
    ~SnapshotKeyTrigger();
    SnapshotKeyTrigger(VideoStream& stream, Format format);

private:
    void detach();

    virtual void run_controller_input(ControllerInputState& state) override;

private:
    VideoStream& m_stream;
    Format m_format;
};


std::string to_format_string(Format format);

// takes a snapshot of the screen and saves it to the given folder_name
void dump_snapshot(
    VideoStream& stream,
    std::string folder_name = "ScreenshotDumper",
    std::string format = ".png"
);

}
}
#endif



