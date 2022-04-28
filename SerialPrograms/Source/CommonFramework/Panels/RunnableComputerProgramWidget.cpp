/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exceptions.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "RunnableComputerProgramWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



RunnableComputerProgramWidget::~RunnableComputerProgramWidget(){
    if (m_scope){
        m_scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
    }
    join_program_thread();
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
    ProgramInfo info(
        program_identifier,
        instance.descriptor().category(),
        instance.descriptor().display_name(),
        timestamp()
    );
    ProgramEnvironment env(info, m_logger, nullptr, nullptr);
    connect(
        &env, &ProgramEnvironment::set_status,
        this, &RunnableComputerProgramWidget::status_update
    );

    CancellableHolder<CancellableScope> scope;
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = &scope;
    }
    try{
        m_logger.log("<b>Starting Program: " + program_identifier + "</b>");
        instance.program(env, scope);
        m_logger.log("Ending Program...");
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
    }catch (OperationCancelledException&){
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException&){
    }catch (InvalidConnectionStateException&){
    }catch (Exception& e){
        emit signal_error(QString::fromStdString(e.to_str()));
    }catch (...){
        std::lock_guard<std::mutex> lg(m_lock);
        m_scope = nullptr;
    }

    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    m_logger.log("Stopping Program...");

    emit signal_reset();
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    m_logger.log("Now in STOPPED state.");
//    cout << "Now in STOPPED state." << endl;
}



}
