/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "RunnableComputerProgram.h"

namespace PokemonAutomation{


RunnableComputerProgramDescriptor::RunnableComputerProgramDescriptor(
    std::string identifier,
    QString category, QString display_name,
    QString doc_link,
    QString description
)
    : RunnablePanelDescriptor(
        Qt::darkCyan,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
{}




QWidget* RunnableComputerProgramInstance::make_widget(QWidget& parent, PanelListener& listener){
    return RunnableComputerProgramWidget::make(parent, *this, listener);
}



RunnableComputerProgramWidget::~RunnableComputerProgramWidget(){
    on_destruct_stop();
}

RunnableComputerProgramWidget* RunnableComputerProgramWidget::make(
    QWidget& parent,
    RunnableComputerProgramInstance& instance,
    PanelListener& listener
){
    RunnableComputerProgramWidget* widget = new RunnableComputerProgramWidget(parent, instance, listener);
    widget->construct();
    widget->m_state.store(ProgramState::STOPPED, std::memory_order_release);
    widget->update_ui_after_program_state_change();
    return widget;
}
void RunnableComputerProgramWidget::run_program(){
    if (m_state.load(std::memory_order_acquire) != ProgramState::RUNNING){
        return;
    }

    RunnableComputerProgramInstance& instance = static_cast<RunnableComputerProgramInstance&>(m_instance);

    const std::string& program_identifier = instance.descriptor().identifier();

    ProgramEnvironment env(
        ProgramInfo(
            program_identifier,
            instance.descriptor().category(),
            instance.descriptor().display_name(),
            timestamp()
        ),
        m_logger, nullptr, nullptr
    );
    connect(
        this, &RunnableComputerProgramWidget::signal_cancel,
        &env, [&]{
            m_state.store(ProgramState::STOPPING, std::memory_order_release);
            env.signal_stop();
        },
        Qt::DirectConnection
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &RunnableComputerProgramWidget::set_status
    );

    try{
        m_logger.log("<b>Starting Program: " + program_identifier + "</b>");
        instance.program(env);
        m_logger.log("Ending Program...");
    }catch (CancelledException&){
    }catch (StringException& e){
        signal_error(e.message_qt());
    }

    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    m_logger.log("Stopping Program...");

    signal_reset();
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    m_logger.log("Now in STOPPED state.");
//    cout << "Now in STOPPED state." << endl;
}



}
