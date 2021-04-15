/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <QJsonObject>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "ClientSource/Connection/PABotBase.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "RunnableSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


using std::cout;
using std::endl;

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

RunnableProgram::RunnableProgram(
    FeedbackType feedback,
    PABotBaseLevel min_pabotbase_level,
    QString name,
    QString doc_link,
    QString description
)
    : RightPanel(pick_color(feedback, min_pabotbase_level), name, std::move(doc_link), description)
    , m_feedback(feedback)
    , m_min_pabotbase_level(min_pabotbase_level)
    , m_setup(nullptr)
{}
void RunnableProgram::from_json(const QJsonValue& json){
    const QJsonObject& obj = json_get_object_nothrow(json.toObject(), m_name);
    m_setup->load_json(json_get_value_nothrow(obj, "SwitchSetup"));
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            item.first->load_json(json_get_value_nothrow(obj, item.second));
        }
    }
}
QJsonValue RunnableProgram::to_json() const{
    QJsonObject obj;
    obj.insert("SwitchSetup", m_setup->to_json());
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            obj.insert(item.second, item.first->to_json());
        }
    }
    return obj;
}

bool RunnableProgram::is_valid() const{
    for (const auto& item : m_options){
        if (!item.first->is_valid()){
            return false;
        }
    }
    return true;
}
void RunnableProgram::restore_defaults(){
    for (const auto& item : m_options){
        item.first->restore_defaults();
    }
}

QWidget* RunnableProgram::make_ui(MainWindow& window){
    RunnableProgramUI* widget = new RunnableProgramUI(*this, window);
    widget->construct();
    return widget;
}

RunnableProgramUI::RunnableProgramUI(RunnableProgram& factory, MainWindow& window)
    : RightPanelUI(factory)
    , m_name(factory.name())
    , m_window(window)
    , m_logger(window.output_window(), "Program")
    , m_setup(nullptr)
    , m_status_bar(nullptr)
    , m_start_button(nullptr)
    , m_state(ProgramState::STOPPED)
{}
void RunnableProgramUI::append_description(QWidget& parent, QVBoxLayout& layout){
    RunnableProgram& factory = static_cast<RunnableProgram&>(m_factory);

    QLabel* text = nullptr;
    switch (factory.m_feedback){
    case FeedbackType::NONE:
        text = new QLabel(
            "<font color=\"purple\">(This program does not use feedback. It can run without video input.</font>)",
            &parent
        );
        break;
    case FeedbackType::OPTIONAL_:
        text = new QLabel(
            "<font color=\"purple\">(This program will use video feedback if it is available. Video input is not required.</font>)",
            &parent
        );
        break;
    case FeedbackType::REQUIRED:
        text = new QLabel(
            "<font color=\"green\">(This program requires video feedback. Please make sure you choose the correct capture device.</font>)",
            &parent
        );
        break;
    }
    layout.addWidget(text);
    text->setWordWrap(true);

    switch (factory.m_min_pabotbase_level){
    case PABotBaseLevel::NOT_PABOTBASE:
        break;
    case PABotBaseLevel::PABOTBASE_12KB:{
#if 0
        QLabel* text = new QLabel(
            "<font color=\"blue\">(This program will run on both Arduino Uno R3 and Teensy 2.0.</font>)",
            &parent
        );
        layout.addWidget(text);
        text->setWordWrap(true);
#endif
        break;
    }case PABotBaseLevel::PABOTBASE_31KB:{
        QLabel* text = new QLabel(
            "<font color=\"red\">(This program requires a Teensy or higher. PABotBase for Arduino Uno R3 does not have all the features required by this program.)</font>",
            &parent
        );
        layout.addWidget(text);
        text->setWordWrap(true);
        break;
    }
    }
}
void RunnableProgramUI::make_body(QWidget& parent, QVBoxLayout& layout){
    QScrollArea* scroll = new QScrollArea(&parent);
    layout.addWidget(scroll);
    scroll->setWidgetResizable(true);

    QWidget* options_widget = new QWidget(scroll);
    (new QVBoxLayout(scroll))->addWidget(options_widget);
    scroll->setWidget(options_widget);

    QVBoxLayout* options_layout = new QVBoxLayout(options_widget);
    options_layout->setAlignment(Qt::AlignTop);


    RunnableProgram& factory = static_cast<RunnableProgram&>(m_factory);
    m_setup = factory.m_setup->make_ui(*options_widget, m_window.output_window());
    options_layout->addWidget(m_setup);
    for (auto& item : factory.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        options_layout->addWidget(m_options.back()->widget());
    }
    RightPanelUI::connect(
        m_setup, &SwitchSetup::on_state_changed,
        this, &RunnableProgramUI::update_ui
    );

    m_status_bar = new QLabel(&parent);
    m_status_bar->setVisible(false);
    m_status_bar->setAlignment(Qt::AlignCenter);
    layout.addWidget(m_status_bar);
//    m_status_bar->setText("<b>Encounters: 1,267 - Corrections: 0 - Star Shinies: 1 - Square Shinies: 0</b>");
    QFont font = m_status_bar->font();
//    cout << font.pointSize() << endl;
    int font_size = font.pointSize();
    font.setPointSize(font_size + font_size / 2);
    m_status_bar->setFont(font);

    QGroupBox* actions_widget = new QGroupBox("Actions", &parent);
    layout.addWidget(actions_widget);

    QHBoxLayout* action_layout = new QHBoxLayout(actions_widget);
    action_layout->setMargin(0);

    {
        m_start_button = new QPushButton("Start Program!", &parent);
        action_layout->addWidget(m_start_button, 2);
        QFont font = m_start_button->font();
        font.setPointSize(16);
        m_start_button->setFont(font);
    }
    {
        m_default_button = new QPushButton("Restore Defaults", &parent);
        action_layout->addWidget(m_default_button, 1);
        QFont font = m_default_button->font();
        font.setPointSize(16);
        m_default_button->setFont(font);
    }

    update_ui();
    connect(
        this, &RunnableProgramUI::signal_reset,
        this, [=]{
            reset_connections();
            update_ui();
        }
    );
    connect(
        this, &RunnableProgramUI::signal_error,
        this, [](QString message){
            QMessageBox box;
            box.critical(nullptr, "Error", message);
        }
    );
    connect(
        m_start_button, &QPushButton::clicked,
        this, [=](bool){
            switch (m_state.load(std::memory_order_acquire)){
            case ProgramState::STOPPED:
                if (!settings_valid()){
                    QMessageBox box;
                    box.critical(nullptr, "Error", "Settings are not valid.");
                    return;
                }
                if (m_thread.joinable()){
                    m_thread.join();
                }
//                m_window.open_output_window();
                m_state.store(ProgramState::RUNNING, std::memory_order_release);
                m_thread = std::thread(&RunnableProgramUI::run_program, this);
                break;
            case ProgramState::RUNNING:
            case ProgramState::FINISHED:
                m_state.store(ProgramState::STOPPING, std::memory_order_release);
                on_stop();
                break;
            case ProgramState::STOPPING:
                break;
            }
            update_ui();
        }
    );
    connect(
        m_default_button, &QPushButton::clicked,
        this, [=](bool){
            restore_defaults();
        }
    );
}
RunnableProgramUI::~RunnableProgramUI(){
    stop();
}
void RunnableProgramUI::stop(){
    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    on_stop();
    if (m_thread.joinable()){
        m_thread.join();
    }
}

bool RunnableProgramUI::settings_valid() const{
    RunnableProgram& factory = static_cast<RunnableProgram&>(m_factory);
    return factory.is_valid() && m_setup && m_setup->serial_ok();
}
void RunnableProgramUI::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}
ProgramState RunnableProgramUI::update_ui(){
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_start_button == nullptr){
        return state;
    }
    m_start_button->setEnabled(state != ProgramState::STOPPING);
    switch (state){
    case ProgramState::STOPPED:
        m_start_button->setText("Start Program...");
//        m_start_button->setEnabled(settings_valid());
        m_window.left_panel_enabled(true);
        break;
    case ProgramState::RUNNING:
        m_start_button->setText("Stop Program...");
        m_window.left_panel_enabled(false);
        break;
    case ProgramState::FINISHED:
        m_start_button->setText("Program Finished! Click to stop.");
        m_window.left_panel_enabled(false);
        break;
    case ProgramState::STOPPING:
        m_start_button->setText("Stopping Program...");
        m_window.left_panel_enabled(false);
        break;
    }

    if (m_setup) m_setup->update_ui(state);

    bool enabled = state == ProgramState::STOPPED;
    m_default_button->setEnabled(enabled);
    for (ConfigOptionUI* option : m_options){
        option->widget()->setEnabled(enabled);
    }

    return state;
}
void RunnableProgramUI::on_stop(){
    signal_cancel();
    if (m_setup) m_setup->stop_serial();
}
void RunnableProgramUI::reset_connections(){
    if (m_setup) m_setup->reset_serial();
}
void RunnableProgramUI::set_status(QString status){
    if (status.size() <= 0){
        m_status_bar->setVisible(false);
        m_status_bar->setText(status);
    }else{
        m_status_bar->setText(status);
        m_status_bar->setVisible(true);
    }
}

void RunnableProgramUI::show_stats_warning() const{
    QMessageBox box;
    box.critical(
        nullptr,
        "Error",
        "Unable to update stats file. You will need to do this manually."
    );
}

void RunnableProgramUI::run_program(){
    if (m_state.load(std::memory_order_acquire) != ProgramState::RUNNING){
        return;
    }

    RunnableProgram& factory = static_cast<RunnableProgram&>(m_factory);
    std::unique_ptr<StatsTracker> current_stats = factory.make_stats();
    std::unique_ptr<StatsTracker> historial_stats;

    std::string program_name = m_name.toUtf8().data();

    //  Aggregate historical stats.
    if (current_stats){
        StatSet stat_sets;
        stat_sets.open_from_file(settings.stats_file);
        StatList& list = stat_sets[program_name];
        if (list.size() != 0){
            historial_stats = factory.make_stats();
            list.aggregate(*historial_stats);
        }
    }


    try{
        m_logger.log("<b>Starting Program: " + m_name + "</b>");
        program(current_stats.get(), historial_stats.get());
        m_setup->wait_for_all_requests();
        m_logger.log("Ending Program...");
    }catch (PokemonAutomation::CancelledException&){
        m_logger.log("Stopping Program...");
    }catch (const char* str){
        signal_error(str);
    }


    //  Update historical stats.
    if (current_stats){
        bool ok = StatSet::update_file(settings.stats_file, program_name, *current_stats);
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
    }


    m_logger.log("Entering STOPPED state.");
    m_state.store(ProgramState::STOPPED, std::memory_order_release);
    signal_reset();
    m_logger.log("Now in STOPPED state.");
//    cout << "Now in STOPPED state." << endl;
}

BotBase& RunnableProgramUI::sanitize_botbase(BotBase* botbase){
    if (botbase != nullptr){
        return *botbase;
    }
    throw StringException("Cannot Start: Serial connection not ready.");
}














}
}





