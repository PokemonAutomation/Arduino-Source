/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H

#include "NintendoSwitch_RunnableProgramWidget.h"
#include "NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SingleSwitchProgramWidget final : public RunnableSwitchProgramWidget{
public:
    static SingleSwitchProgramWidget* make(
        QWidget& parent,
        SingleSwitchProgramInstance& instance,
        PanelListener& listener
    );

    SwitchSystem& system(){
        return *static_cast<SwitchSystem*>(m_setup);
    }

private:
    using RunnableSwitchProgramWidget::RunnableSwitchProgramWidget;
    virtual ~SingleSwitchProgramWidget();

private:
    virtual void run_program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ) override;

private:
    friend class SingleSwitchProgramInstance;
};



}
}
#endif
