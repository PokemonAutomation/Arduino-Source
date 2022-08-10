/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H

#include "NintendoSwitch_RunnableProgramWidget.h"
#include "NintendoSwitch_SwitchSystemWidget.h"
#include "NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch_SingleSwitchProgramSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SingleSwitchProgramWidget final : public RunnableSwitchProgramWidget{
public:
    static SingleSwitchProgramWidget* make(
        QWidget& parent,
        SingleSwitchProgramInstance& instance,
        PanelHolder& holder
    );

    SwitchSystemWidget& system(){
        return *static_cast<SwitchSystemWidget*>(m_setup);
    }

private:
    using RunnableSwitchProgramWidget::RunnableSwitchProgramWidget;
    virtual ~SingleSwitchProgramWidget();

private:
    virtual void run_switch_program(const ProgramInfo& info) override;

private:
    friend class SingleSwitchProgramInstance;
};






class SingleSwitchProgramWidget2 : public QWidget, private ProgramSession::Listener{
public:
    ~SingleSwitchProgramWidget2();
    SingleSwitchProgramWidget2(
        QWidget& parent,
        SingleSwitchProgramOption& option,
        PanelHolder& holder
    );


private:
    virtual void state_change(ProgramState state) override;
    virtual void stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats) override;
    virtual void error(const std::string& message) override;



private:
    PanelHolder& m_holder;
    SingleSwitchProgramSession m_session;
    SwitchSystemWidget* m_system;
    ConfigWidget* m_options;
    StatsBar* m_stats_bar;
    RunnablePanelActionBar* m_actions_bar;
};







}
}
#endif
