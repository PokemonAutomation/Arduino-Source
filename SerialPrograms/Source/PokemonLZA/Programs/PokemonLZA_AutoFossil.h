/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Hunt shiny and/or alpha fossils by repeated reviving fossils and check them in box
 */

#ifndef PokemonAutomation_PokemonLZA_AutoFossil_H
#define PokemonAutomation_PokemonLZA_AutoFossil_H

#include "Common/Cpp/Options/ButtonOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class AutoFossil_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoFossil_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class AutoFossil : public SingleSwitchProgramInstance{
public:
    AutoFossil();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void revive_one_fossil(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    void check_fossils_in_box(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t num_boxes);
};




}
}
}
#endif
