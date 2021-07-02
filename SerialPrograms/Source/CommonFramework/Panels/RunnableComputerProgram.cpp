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
    QString display_name,
    QString doc_link,
    QString description
)
    : RunnablePanelDescriptor(
        Qt::darkCyan,
        std::move(identifier),
        std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
{}




QWidget* RunnableComputerProgramInstance::make_widget(QWidget& parent, PanelListener& listener){
    return RunnableComputerProgramWidget::make(parent, *this, listener);
}



RunnableComputerProgramWidget::~RunnableComputerProgramWidget(){
    if (!m_destructing){
        stop();
        m_destructing = true;
    }
}

RunnableComputerProgramWidget* RunnableComputerProgramWidget::make(
    QWidget& parent,
    RunnableComputerProgramInstance& instance,
    PanelListener& listener
){
    RunnableComputerProgramWidget* widget = new RunnableComputerProgramWidget(parent, instance, listener);
    widget->construct();
    return widget;
}
void RunnableComputerProgramWidget::run_program(){
    if (m_state.load(std::memory_order_acquire) != ProgramState::RUNNING){
        return;
    }

    RunnableComputerProgramInstance& instance = static_cast<RunnableComputerProgramInstance&>(m_instance);

    ProgramEnvironment env(m_logger, nullptr, nullptr);
    connect(
        this, &RunnableComputerProgramWidget::signal_cancel,
        &env, [&]{
            env.signal_stop();
        },
        Qt::DirectConnection
    );
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &RunnableComputerProgramWidget::set_status
    );

    try{
        m_logger.log("<b>Starting Program: " + instance.descriptor().identifier() + "</b>");
        instance.program(env);
        m_logger.log("Ending Program...");
    }catch (CancelledException&){
        m_logger.log("Stopping Program...");
    }catch (StringException& e){
        signal_error(e.message_qt());
    }

    m_logger.log("Entering STOPPED state.");
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    signal_reset();
    m_logger.log("Now in STOPPED state.");
}



}
