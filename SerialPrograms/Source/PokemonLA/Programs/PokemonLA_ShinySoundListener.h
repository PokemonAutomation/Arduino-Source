/*  Shiny Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinySoundListener_H
#define PokemonAutomation_PokemonLA_ShinySoundListener_H

#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonLA{

class ShinySoundListener_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    ShinySoundListener_Descriptor();
};


class ShinySoundListener : public SingleSwitchProgramInstance{
public:
    ShinySoundListener(const ShinySoundListener_Descriptor& descriptor);

//    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

//private:
//    struct Stats;

private:
    // GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
};




}
}
}
#endif
