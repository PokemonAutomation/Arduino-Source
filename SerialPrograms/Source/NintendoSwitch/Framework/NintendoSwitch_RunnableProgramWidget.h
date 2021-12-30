/*  Runnable Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RunnableProgramWidget_H
#define PokemonAutomation_NintendoSwitch_RunnableProgramWidget_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Panels/RunnablePanelWidget.h"
#include "NintendoSwitch_RunnableProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

class SwitchSetupWidget;


class RunnableSwitchProgramWidget : public RunnablePanelWidget{
public:
    virtual ~RunnableSwitchProgramWidget();

    SwitchSetupWidget& setup(){ return *m_setup; }

protected:
    RunnableSwitchProgramWidget(
        QWidget& parent,
        RunnableSwitchProgramInstance& instance,
        PanelListener& listener
    );
    void construct();
    virtual QWidget* make_header(QWidget& parent) override;
    virtual QWidget* make_body(QWidget& parent) override;
    virtual QWidget* make_actions(QWidget& parent) override;

protected:
    virtual QString check_validity() const override;

    virtual void update_ui_after_program_state_change() override;

    virtual void on_stop() override;

    virtual void run_program() override final;
    virtual void run_program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ) = 0;

protected:
    static BotBase& sanitize_botbase(BotBase* botbase);

protected:
    friend class RunnableSwitchProgramInstance;

    const std::string& m_program_name;

    SwitchSetupWidget* m_setup;
};




}
}
#endif
