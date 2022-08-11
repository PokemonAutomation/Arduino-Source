/*  Multi-Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgramWidget_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgramWidget_H

#include "CommonFramework/Panels/PanelElements.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramSession.h"
#include "NintendoSwitch_MultiSwitchSystemWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class MultiSwitchProgramWidget2 : public QWidget, private ProgramSession::Listener, private MultiSwitchProgramSession::Listener{
public:
    ~MultiSwitchProgramWidget2();
    MultiSwitchProgramWidget2(
        QWidget& parent,
        MultiSwitchProgramOption& option,
        PanelHolder& holder
    );

private:
    virtual void state_change(ProgramState state) override;
    virtual void stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats) override;
    virtual void error(const std::string& message) override;

    virtual void redraw_options() override;

private:
    PanelHolder& m_holder;
    MultiSwitchProgramSession m_session;
    MultiSwitchSystemWidget* m_system;
    ConfigWidget* m_options;
    StatsBar* m_stats_bar;
    RunnablePanelActionBar* m_actions_bar;
};






}
}
#endif
