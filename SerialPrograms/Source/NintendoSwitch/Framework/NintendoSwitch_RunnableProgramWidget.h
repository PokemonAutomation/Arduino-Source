/*  Runnable Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_RunnableProgramWidget_H
#define PokemonAutomation_NintendoSwitch_RunnableProgramWidget_H

#include "Common/Cpp/SpinLock.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
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
        PanelHolder& holder
    );
    void construct();
    virtual CollapsibleGroupBox* make_header(QWidget& parent) override;
    virtual QWidget* make_body(QWidget& parent) override;
    virtual QWidget* make_actions(QWidget& parent) override;

protected:
    virtual std::string check_validity() const override;

    virtual void update_ui_after_program_state_change() override;

    virtual bool request_program_stop() override;

    virtual void run_program() override final;
    virtual void run_switch_program(const ProgramInfo& info) = 0;

protected:
    BotBase& sanitize_botbase(BotBase* botbase);

protected:
    friend class RunnableSwitchProgramInstance;

    const std::string& m_program_name;

    SwitchSetupWidget* m_setup;
};




}
}
#endif
