#pragma once
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
class OverworldLightingTest_Descriptor : public SingleSwitchProgramDescriptor{
public: OverworldLightingTest_Descriptor();
};
class OverworldLightingTest : public SingleSwitchProgramInstance{
public:
    OverworldLightingTest();
    void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
private:
    StringSelectOption MATCHER;
    SimpleIntegerOption<uint8_t> START_HOUR;
    SimpleIntegerOption<uint16_t> SETTLE_SECONDS;
};
}}}
