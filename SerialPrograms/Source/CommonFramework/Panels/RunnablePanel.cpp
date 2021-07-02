/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include "Common/Cpp/PanicDump.h"
#include "Common/Qt/QtJsonTools.h"
#include "RunnablePanel.h"

namespace PokemonAutomation{



void RunnablePanelInstance::from_json(const QJsonValue& json){
    const QJsonObject& obj = json.toObject();
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            item.first->load_json(json_get_value_nothrow(obj, item.second));
        }
    }
}
QJsonValue RunnablePanelInstance::to_json() const{
    QJsonObject obj;
    for (auto& item : m_options){
        if (!item.second.isEmpty()){
            obj.insert(item.second, item.first->to_json());
        }
    }
    return obj;
}

bool RunnablePanelInstance::is_valid() const{
    for (const auto& item : m_options){
        if (!item.first->is_valid()){
            return false;
        }
    }
    return true;
}
void RunnablePanelInstance::restore_defaults(){
    for (const auto& item : m_options){
        item.first->restore_defaults();
    }
}



void RunnablePanelWidget::stop(){
    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    on_stop();
    if (m_thread.joinable()){
        m_thread.join();
    }
}
RunnablePanelWidget::~RunnablePanelWidget(){
    if (!m_destructing){
        stop();
        m_destructing = true;
    }
}



RunnablePanelWidget::RunnablePanelWidget(
    QWidget& parent,
    RunnablePanelInstance& instance,
    PanelListener& listener
)
    : PanelWidget(parent, instance, listener)
    , m_logger(listener.output_window(), "Program")
    , m_status_bar(nullptr)
    , m_start_button(nullptr)
    , m_state(ProgramState::STOPPED)
{}
void RunnablePanelWidget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(make_header(*this));

    QScrollArea* scroll = new QScrollArea(this);
    layout->addWidget(scroll);
    scroll->setWidgetResizable(true);

    scroll->setWidget(make_options(*scroll));
    m_status_bar = make_status_bar(*this);
    layout->addWidget(m_status_bar);
    layout->addWidget(make_actions(*this));
}
QWidget* RunnablePanelWidget::make_options(QWidget& parent){
    QWidget* options_widget = new QWidget(&parent);

    QVBoxLayout* options_layout = new QVBoxLayout(options_widget);
    options_layout->setAlignment(Qt::AlignTop);

    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    for (auto& item : instance.m_options){
        m_options.emplace_back(item.first->make_ui(parent));
        options_layout->addWidget(m_options.back()->widget());
    }

    return options_widget;
}
QLabel* RunnablePanelWidget::make_status_bar(QWidget& parent){
    QLabel* status_bar = new QLabel(&parent);
    status_bar = new QLabel(&parent);
    status_bar->setVisible(false);
    status_bar->setAlignment(Qt::AlignCenter);
//    status_bar->setText("<b>Encounters: 1,267 - Corrections: 0 - Star Shinies: 1 - Square Shinies: 0</b>");
    QFont font = status_bar->font();
    font.setPointSize(10);
    status_bar->setFont(font);
    return status_bar;
}
QWidget* RunnablePanelWidget::make_actions(QWidget& parent){
    QGroupBox* actions_widget = new QGroupBox("Actions", &parent);

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
        this, &RunnablePanelWidget::signal_reset,
        this, [=]{ update_ui(); }
    );
    connect(
        this, &RunnablePanelWidget::signal_error,
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
                m_thread = std::thread(
                    run_with_catch,
                    "RunnablePanelWidget::run_program()",
                    [=]{ run_program(); }
                );
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

    return actions_widget;
}



bool RunnablePanelWidget::settings_valid() const{
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    return instance.is_valid();
}
void RunnablePanelWidget::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}
void RunnablePanelWidget::update_ui(){
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_start_button == nullptr){
        return;
    }
    m_start_button->setEnabled(state != ProgramState::STOPPING);
    switch (state){
    case ProgramState::STOPPED:
        m_start_button->setText("Start Program...");
//        m_start_button->setEnabled(settings_valid());
        m_listener.on_idle(m_instance);
        break;
    case ProgramState::RUNNING:
        m_start_button->setText("Stop Program...");
        m_listener.on_busy(m_instance);
        break;
    case ProgramState::FINISHED:
        m_start_button->setText("Program Finished! Click to stop.");
        m_listener.on_busy(m_instance);
        break;
    case ProgramState::STOPPING:
        m_start_button->setText("Stopping Program...");
        m_listener.on_busy(m_instance);
        break;
    }

    bool enabled = state == ProgramState::STOPPED;
    m_default_button->setEnabled(enabled);
    for (ConfigOptionUI* option : m_options){
        option->widget()->setEnabled(enabled);
    }
}


void RunnablePanelWidget::set_status(QString status){
    if (status.size() <= 0){
        m_status_bar->setVisible(false);
        m_status_bar->setText(status);
    }else{
        m_status_bar->setText(status);
        m_status_bar->setVisible(true);
    }
}

void RunnablePanelWidget::on_stop(){
    signal_cancel();
}








}
