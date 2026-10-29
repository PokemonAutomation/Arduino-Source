#pragma once
#include <array>
#include <memory>
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
class OverworldLightingTest_Descriptor : public SingleSwitchProgramDescriptor{
public: OverworldLightingTest_Descriptor();
};
class OverworldLightingTest : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    using Descriptor = OverworldLightingTest_Descriptor;
    ~OverworldLightingTest();
    OverworldLightingTest();
    std::string check_validity() const override;
    void load_json(const JsonValue& json) override;
    void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
private:
    void on_config_value_changed(void* object) override;
    IntegerEnumDropdownOption SEGMENT;
    std::array<std::unique_ptr<StringSelectOption>,10> MATCHERS;
    SimpleIntegerOption<uint8_t> START_HOUR;
    SimpleIntegerOption<uint16_t> SETTLE_SECONDS;
};
}}}
