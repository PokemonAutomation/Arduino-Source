/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Qt/QtJsonTools.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "RunnableSwitchProgram.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



QColor pick_color(FeedbackType feedback, PABotBaseLevel size){
    switch (size){
    case PABotBaseLevel::NOT_PABOTBASE:
        return QColor();
    case PABotBaseLevel::PABOTBASE_12KB:
        return feedback == FeedbackType::REQUIRED ? QColor("green") : QColor("blue");
    case PABotBaseLevel::PABOTBASE_31KB:
        return feedback == FeedbackType::REQUIRED ? QColor("purple") : QColor("red");
    }
    return QColor();
}
RunnableSwitchProgramDescriptor::RunnableSwitchProgramDescriptor(
    std::string identifier,
    QString display_name,
    QString doc_link,
    QString description,
    FeedbackType feedback,
    PABotBaseLevel min_pabotbase_level
)
    : RunnablePanelDescriptor(
        pick_color(feedback, min_pabotbase_level),
        std::move(identifier),
        std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
    , m_feedback(feedback)
    , m_min_pabotbase_level(min_pabotbase_level)
{}




void RunnableSwitchProgramInstance::from_json(const QJsonValue& json){
    const QJsonObject& obj = json.toObject();
    m_setup->load_json(json_get_value_nothrow(obj, "SwitchSetup"));
    RunnablePanelInstance::from_json(json);
}
QJsonValue RunnableSwitchProgramInstance::to_json() const{
    QJsonObject obj = RunnablePanelInstance::to_json().toObject();
    obj.insert("SwitchSetup", m_setup->to_json());
    return obj;
}



RunnableSwitchProgramWidget::~RunnableSwitchProgramWidget(){
    if (!m_destructing){
        stop();
        m_destructing = true;
    }
}

RunnableSwitchProgramWidget::RunnableSwitchProgramWidget(
    QWidget& parent,
    RunnableSwitchProgramInstance& instance,
    PanelListener& listener
)
    : RunnablePanelWidget(parent, instance, listener)
    , m_setup(nullptr)
{}
void RunnableSwitchProgramWidget::construct(){
    RunnablePanelWidget::construct();
    update_historical_stats();
}
QWidget* RunnableSwitchProgramWidget::make_header(QWidget& parent){
    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    QWidget* header = PanelWidget::make_header(parent);
    QLayout* layout = header->layout();

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
            "<font color=\"green\">(This program requires video feedback. Please make sure you choose the correct capture device.</font>)",
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
        QLabel* text = new QLabel(
            "<font color=\"red\">(This program requires a Teensy or higher. PABotBase for Arduino Uno R3 does not have all the features required by this program.)</font>",
            header
        );
        text->setWordWrap(true);
        layout->addWidget(text);
        break;
    }
    }

    return header;
}
QWidget* RunnableSwitchProgramWidget::make_options(QWidget& parent){
    QWidget* options_widget = RunnablePanelWidget::make_options(parent);

    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    m_setup = instance.m_setup->make_ui(*options_widget, m_listener.output_window());
    static_cast<QVBoxLayout*>(options_widget->layout())->insertWidget(0, m_setup);

    return options_widget;
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


bool RunnableSwitchProgramWidget::settings_valid() const{
    return RunnablePanelWidget::settings_valid() && m_setup && m_setup->serial_ok();
}
void RunnableSwitchProgramWidget::update_ui(){
    RunnablePanelWidget::update_ui();
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_setup) m_setup->update_ui(state);
}
void RunnableSwitchProgramWidget::update_historical_stats(){
    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    m_stats = instance.make_stats();
    if (m_stats){
        StatSet stats;
        stats.open_from_file(PERSISTENT_SETTINGS().stats_file);
        const std::string& identifier = instance.descriptor().identifier();
        StatList& list = stats[identifier];
        if (list.size() != 0){
            list.aggregate(*m_stats);
        }
        m_status_bar->setText(m_stats->to_str().c_str());
        m_status_bar->setVisible(true);
    }
}

void RunnableSwitchProgramWidget::on_stop(){
    RunnablePanelWidget::on_stop();
    if (m_setup){
        m_setup->stop_serial();
    }
}

void RunnableSwitchProgramWidget::run_program(){
    if (m_state.load(std::memory_order_acquire) != ProgramState::RUNNING){
        return;
    }

    RunnableSwitchProgramInstance& instance = static_cast<RunnableSwitchProgramInstance&>(m_instance);
    std::unique_ptr<StatsTracker> current_stats = instance.make_stats();

//    std::string program_name = instance.descriptor().display_name().toUtf8().data();
    update_historical_stats();

    try{
        m_logger.log("<b>Starting Program: " + instance.descriptor().identifier() + "</b>");
        run_program(current_stats.get(), m_stats.get());
        m_setup->wait_for_all_requests();
        m_logger.log("Ending Program...");
    }catch (CancelledException&){
        m_logger.log("Stopping Program...");
    }catch (StringException& e){
        signal_error(e.message_qt());
    }


    //  Update historical stats.
    if (current_stats){
        bool ok = StatSet::update_file(
            PERSISTENT_SETTINGS().stats_file,
            instance.descriptor().identifier(),
            *current_stats
        );
        if (ok){
            m_logger.log("Stats successfully saved!", "Blue");
        }else{
            m_logger.log("Unable to save stats.", "Red");
            QMetaObject::invokeMethod(
                this,
                "show_stats_warning",
                Qt::AutoConnection
            );
//            show_stats_warning();
        }
//        settings.stat_sets.open_from_file(settings.stats_file);
    }


    m_logger.log("Entering STOPPED state.");
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    signal_reset();
    m_logger.log("Now in STOPPED state.");
//    cout << "Now in STOPPED state." << endl;
}



BotBase& RunnableSwitchProgramWidget::sanitize_botbase(BotBase* botbase){
    if (botbase != nullptr){
        return *botbase;
    }
    PA_THROW_StringException("Cannot Start: Serial connection not ready.");
}





}
}





