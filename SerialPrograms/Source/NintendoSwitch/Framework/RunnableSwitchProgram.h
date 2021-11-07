/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnableProgram_H
#define PokemonAutomation_RunnableProgram_H

#include <thread>
#include <QLabel>
#include <QPushButton>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Panels/Panel.h"
#include "CommonFramework/Panels/RunnablePanel.h"
#include "SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class RunnableSwitchProgramDescriptor : public RunnablePanelDescriptor{
public:
    RunnableSwitchProgramDescriptor(
        std::string identifier,
        QString display_name,
        QString doc_link,
        QString description,
        FeedbackType feedback,
        PABotBaseLevel min_pabotbase_level
    );

    FeedbackType feedback() const{ return m_feedback; }
    PABotBaseLevel min_pabotbase_level() const{ return m_min_pabotbase_level; }

protected:
    const FeedbackType m_feedback;
    const PABotBaseLevel m_min_pabotbase_level;
};



class RunnableSwitchProgramInstance : public RunnablePanelInstance{
public:
    using RunnablePanelInstance::RunnablePanelInstance;

    const RunnableSwitchProgramDescriptor& descriptor() const{
        return static_cast<const RunnableSwitchProgramDescriptor&>(m_descriptor);
    }

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

protected:
    friend class RunnableSwitchProgramWidget;

    SwitchSetupFactory* m_setup = nullptr;
};



class RunnableSwitchProgramWidget : public RunnablePanelWidget{
public:
    virtual ~RunnableSwitchProgramWidget();

    SwitchSetup& setup(){ return *m_setup; }

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

    SwitchSetup* m_setup;
};





}
}
#endif





