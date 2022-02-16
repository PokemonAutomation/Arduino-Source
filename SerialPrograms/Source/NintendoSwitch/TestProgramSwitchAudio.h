/*  Test Program Switch Audio
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_TestProgramSwitchAudio_H
#define PokemonAutomation_NintendoSwitch_TestProgramSwitchAudio_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class TestProgramAudio_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TestProgramAudio_Descriptor();
};


class TestProgramAudio : public SingleSwitchProgramInstance{
public:
    TestProgramAudio(const TestProgramAudio_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
};


}
}
#endif
