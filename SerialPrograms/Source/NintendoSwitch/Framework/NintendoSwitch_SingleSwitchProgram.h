/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgram_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch_SwitchSystem.h"
#include "NintendoSwitch_RunnableProgram.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{


class SingleSwitchProgramEnvironment : public ProgramEnvironment{
public:
    ConsoleHandle console;

private:
    friend class SingleSwitchProgramWidget;
    template <class... Args>
    SingleSwitchProgramEnvironment(
        const ProgramInfo& program_info,
        CancellableScope& scope,
        LoggerQt& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        Args&&... args
    )
        : ProgramEnvironment(program_info, logger, current_stats, historical_stats)
        , console(0, std::forward<Args>(args)...)
    {
        console.initialize_inference_threads(scope, inference_dispatcher());
    }
};



class SingleSwitchProgramInstance : public RunnableSwitchProgramInstance{
public:
    SingleSwitchProgramInstance(const RunnableSwitchProgramDescriptor& descriptor);
    virtual QWidget* make_widget(QWidget& parent, PanelHolder& holder) override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) = 0;

private:
    friend class SingleSwitchProgramWidget;

    SwitchSystemFactory m_switch;
};




}
}
#endif

