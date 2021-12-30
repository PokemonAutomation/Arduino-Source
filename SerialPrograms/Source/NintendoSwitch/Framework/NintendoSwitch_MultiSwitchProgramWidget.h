/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramWidget_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramWidget_H

#include "NintendoSwitch_RunnableProgramWidget.h"
#include "NintendoSwitch_MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class MultiSwitchProgramWidget final : public RunnableSwitchProgramWidget{
public:
    static MultiSwitchProgramWidget* make(
        QWidget& parent,
        MultiSwitchProgramInstance& instance,
        PanelListener& listener
    );

    size_t system_count() const{
        return static_cast<MultiSwitchSystem&>(*m_setup).switch_count();
    }
    SwitchSystem& system(size_t index){
        return static_cast<MultiSwitchSystem&>(*m_setup)[index];
    }

private:
    using RunnableSwitchProgramWidget::RunnableSwitchProgramWidget;
    virtual ~MultiSwitchProgramWidget();

private:
    virtual void run_program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ) override;

private:
    friend class MultiSwitchProgramInstance;
};






}
}
#endif
