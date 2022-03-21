/*  Runnable Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch_SwitchSetupWidget.h"
#include "NintendoSwitch_RunnableProgramWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




RunnableSwitchProgramWidget::~RunnableSwitchProgramWidget(){
    RunnableSwitchProgramWidget::request_program_stop();
    join_program_thread();
}

RunnableSwitchProgramWidget::RunnableSwitchProgramWidget(
    QWidget& parent,
    RunnableSwitchProgramInstance& instance,
    PanelListener& listener
)
    : RunnablePanelWidget(parent, instance, listener)
    , m_program_name(instance.descriptor().identifier())
    , m_setup(nullptr)
{}
void RunnableSwitchProgramWidget::construct(){
    RunnablePanelWidget::construct();
    load_historical_stats();
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    update_ui_after_program_state_change();
}
CollapsibleGroupBox* RunnableSwitchProgramWidget::make_header(QWidget& parent){
    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    CollapsibleGroupBox* header = PanelWidget::make_header(parent);
    QLayout* layout = header->widget()->layout();

    QLabel* text = nullptr;
    switch (instance.descriptor().feedback()){
    case FeedbackType::NONE:
        text = new QLabel(
            "<font color=\"purple\">(This program does not use feedback. It can run without video input.</font>)",
            header
        );
        break;
    case FeedbackType::OPTIONAL_:
        text = new QLabel(
            "<font color=\"purple\">(This program will use video feedback if it is available. Video input is not required.</font>)",
            header
        );
        break;
    case FeedbackType::REQUIRED:
        text = new QLabel(
            "<font color=\"green\">(This program requires video feedback. Please make sure you choose the correct capture device.)</font>",
            header
        );
        break;
    }
    text->setWordWrap(true);
    layout->addWidget(text);

    switch (instance.descriptor().min_pabotbase_level()){
    case PABotBaseLevel::NOT_PABOTBASE:
        break;
    case PABotBaseLevel::PABOTBASE_12KB:{
#if 0
        QLabel* text = new QLabel(
            "<font color=\"blue\">(This program will run on both Arduino Uno R3 and Teensy 2.0.</font>)",
            header
        );
        text->setWordWrap(true);
        layout->addWidget(text);
#endif
        break;
    }case PABotBaseLevel::PABOTBASE_31KB:{
        QLabel* label = new QLabel(
            "<font color=\"red\">(This program requires a Teensy or higher. PABotBase for Arduino Uno R3 does not have all the features required by this program.)</font>",
            header
        );
        label->setWordWrap(true);
        layout->addWidget(label);
        break;
    }
    }

    return header;
}
QWidget* RunnableSwitchProgramWidget::make_body(QWidget& parent){
    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    m_setup = instance.m_setup->make_ui(parent, m_listener.raw_logger(), m_instance_id);
    return m_setup;
}
QWidget* RunnableSwitchProgramWidget::make_actions(QWidget& parent){
    QWidget* actions_widget = RunnablePanelWidget::make_actions(parent);
    connect(
        this, &RunnableSwitchProgramWidget::signal_reset,
        this, [=]{
            if (m_setup){
                m_setup->reset_serial();
            }
        }
    );
    return actions_widget;
}


QString RunnableSwitchProgramWidget::check_validity() const{
    QString error = RunnablePanelWidget::check_validity();
    if (!error.isEmpty()){
        return error;
    }
    return m_setup && m_setup->serial_ok()
        ? QString()
        : "Switch setup is not ready.";
}
void RunnableSwitchProgramWidget::update_ui_after_program_state_change(){
    RunnablePanelWidget::update_ui_after_program_state_change();
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_setup) m_setup->update_ui(state);
}

bool RunnableSwitchProgramWidget::request_program_stop(){
    if (!RunnablePanelWidget::request_program_stop()){
        return false;
    }
    emit signal_cancel();
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_setup){
        m_setup->stop_serial();
        m_setup->update_ui(state);
    }
    return true;
}

void RunnableSwitchProgramWidget::run_program(){
    if (m_state.load(std::memory_order_acquire) != ProgramState::RUNNING){
        return;
    }

    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_current_stats = instance.make_stats();
    }

    const std::string& program_identifier = instance.descriptor().identifier();

    load_historical_stats();

    try{
        m_logger.log("<b>Starting Program: " + program_identifier + "</b>");
        run_switch_program();
        m_setup->wait_for_all_requests();
        m_logger.log("Ending Program...");
    }catch (ProgramCancelledException&){
    }catch (ProgramFinishedException&){
    }catch (InvalidConnectionStateException&){
    }catch (Exception& e){
        QString message = QString::fromStdString(e.message());
        emit signal_error(message);
        send_program_fatal_error_notification(
            m_logger, instance.NOTIFICATION_ERROR_FATAL,
            ProgramInfo(
                instance.descriptor().identifier(),
                instance.descriptor().category(),
                instance.descriptor().display_name(),
                timestamp()
            ),
            std::move(message),
            m_current_stats ? m_current_stats->to_str() : ""
        );
    }

    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    m_logger.log("Stopping Program...");

    update_historical_stats();

    emit signal_reset();
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    m_logger.log("Now in STOPPED state.");
//    cout << "Now in STOPPED state." << endl;
}



BotBase& RunnableSwitchProgramWidget::sanitize_botbase(BotBase* botbase){
    if (botbase != nullptr){
        return *botbase;
    }
    throw UserSetupError(m_logger, "Cannot Start: Serial connection not ready.");
}



}
}
