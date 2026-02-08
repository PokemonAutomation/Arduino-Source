/*  Turbo Macro
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TurboMacro_H
#define PokemonAutomation_PokemonLZA_TurboMacro_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Controllers/ControllerStateTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class LZA_TurboMacro_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LZA_TurboMacro_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class LZA_TurboMacro : public SingleSwitchProgramInstance{
public:
    LZA_TurboMacro();

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

    void run_table(SingleSwitchProgramEnvironment& env, CancellableScope& scope);

    void run_table_stop_when_shiny_sound(SingleSwitchProgramEnvironment& env, CancellableScope& scope);


private:
    SimpleIntegerOption<uint32_t> LOOP;
    ControllerCommandTables TABLE;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    enum class RunUntilCallback{
        NONE,
        SHINY_SOUND,
    };
    EnumDropdownOption<RunUntilCallback> RUN_UNTIL_CALLBACK;
};



}
}
}
#endif // PokemonAutomation_PokemonLZA_TurboMacro_H
