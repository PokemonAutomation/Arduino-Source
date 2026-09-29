/*  Snapshot Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_SnapshotDumper_H
#define PokemonAutomation_GameConsole_SnapshotDumper_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "ControllerInput/ControllerInput.h"
#include "GameConsole/ConsoleProgram.h"

namespace PokemonAutomation{
namespace GameConsole{


class SnapshotDumper_Descriptor : public ConsoleProgramDescriptor{
public:
    SnapshotDumper_Descriptor();
};

enum class Format{
    PNG,
    JPG,
};

class SnapshotDumper : public ConsoleProgramInstance, public ConfigOption::Listener{
public:
    using Descriptor = SnapshotDumper_Descriptor;
    ~SnapshotDumper();
    SnapshotDumper();

    virtual void program(ConsoleProgramEnvironment& env, CancellableScope& scope) override;

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



