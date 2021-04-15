/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


SingleSwitchProgram::SingleSwitchProgram(
    FeedbackType feedback,
    PABotBaseLevel min_pabotbase,
    QString name,
    QString doc_link,
    QString description
)
    : RunnableProgram(
        feedback, min_pabotbase,
        std::move(name),
        std::move(doc_link),
        std::move(description)
    )
    , m_switch(
        "Switch Settings", "Switch 0",
        min_pabotbase, feedback
    )
{
    m_setup = &m_switch;
}

SingleSwitchProgramUI::SingleSwitchProgramUI(SingleSwitchProgram& factory, MainWindow& window)
    : RunnableProgramUI(factory, window)
{
    this->construct();
}


SingleSwitchProgramUI::~SingleSwitchProgramUI(){ stop(); }

void SingleSwitchProgramUI::program(
    StatsTracker* current_stats,
    const StatsTracker* historical_stats
){
    SwitchSystem* system = static_cast<SwitchSystem*>(m_setup);
    SingleSwitchProgramEnvironment env(
        m_logger,
        current_stats, historical_stats,
        sanitize_botbase(system->botbase()),
        system->camera()
    );
    connect(
        this, &RunnableProgramUI::signal_cancel,
        &env, [&]{
            env.signal_stop();
        },
        Qt::DirectConnection
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, [=](QString status){
            this->set_status(std::move(status));
        }
    );
    PokemonAutomation::global_connection = &(BotBase&)env.console;
    SingleSwitchProgram& factory = static_cast<SingleSwitchProgram&>(m_factory);
    factory.program(env);
}



}
}
