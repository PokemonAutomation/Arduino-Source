/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "MultiSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchProgramEnvironment::MultiSwitchProgramEnvironment(Logger& p_logger, std::vector<ConsoleHandle> p_switches)
    : logger(p_logger)
    , dispatcher(p_switches.size())
    , consoles(std::move(p_switches))
{}

void MultiSwitchProgramEnvironment::run_in_parallel(
    const std::function<void(ConsoleHandle& console)>& func
){
    run_in_parallel(0, consoles.size(), func);
}
void MultiSwitchProgramEnvironment::run_in_parallel(
    size_t s, size_t e,
    const std::function<void(ConsoleHandle& console)>& func
){
    dispatcher.run_in_parallel(
        s, e,
        [&](size_t index){
            func(consoles[index]);
            consoles[index].botbase().wait_for_all_requests();
        }
    );
}



MultiSwitchProgram::MultiSwitchProgram(
    FeedbackType feedback,
    PABotBaseLevel min_pabotbase,
    QString name,
    QString doc_link,
    QString description,
    size_t min_switches,
    size_t max_switches,
    size_t switches
)
    : RunnableProgram(
        feedback, min_pabotbase,
        std::move(name),
        std::move(doc_link),
        std::move(description)
    )
    , m_switches(
        min_pabotbase, feedback,
        min_switches,
        max_switches,
        switches
    )
{
    m_setup = &m_switches;
}


MultiSwitchProgramUI::MultiSwitchProgramUI(MultiSwitchProgram& factory, MainWindow& window)
    : RunnableProgramUI(factory, window)
{
    this->construct();
}
MultiSwitchProgramUI::~MultiSwitchProgramUI(){ stop(); }

void MultiSwitchProgramUI::program(){
    MultiSwitchProgram& factory = static_cast<MultiSwitchProgram&>(m_factory);
    std::vector<ConsoleHandle> switches;
    for (size_t c = 0; c < factory.count(); c++){
        SwitchSystem& system = static_cast<MultiSwitchSystem&>(*m_setup)[c];
        switches.emplace_back(
            c,
            sanitize_botbase(system.botbase()),
            system.camera()
        );
    }
    MultiSwitchProgramEnvironment env(m_logger, std::move(switches));
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
    PokemonAutomation::global_connection = nullptr;
    factory.program(env);
}



}
}
