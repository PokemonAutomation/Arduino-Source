/*  Single Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  This is the Qt Widget implementation of the UI for SwitchSystemSession.
 *
 *  On construction, this class attaches itself to the session it is constructed
 *  with and automatically detaches on destruction. Therefore, this class must
 *  not outlive the session it is constructed with. While not useful, it is also
 *  safe to construct multiple UI classes attached to the same session.
 *
 *  Modifications directly to the session object will automatically update this
 *  UI class. For example, if you use Discord to change the volume of the
 *  audio playback, it will move the slider as shown by this UI.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H
#define PokemonAutomation_NintendoSwitch_SingleSwitchProgramWidget_H

#include "CommonFramework/Panels/UI/PanelElements.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramSession.h"
#include "NintendoSwitch_SwitchSystemWidget.h"

namespace PokemonAutomation{
    struct PanelHolder;
namespace NintendoSwitch{



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
