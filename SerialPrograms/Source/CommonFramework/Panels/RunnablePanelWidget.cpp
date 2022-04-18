/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/PanicDump.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Options/BatchOption/BatchWidget.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "Integrations/ProgramTracker.h"
#include "RunnablePanelWidget.h"

namespace PokemonAutomation{



void RunnablePanelWidget::join_program_thread(){
//    if (m_destructing){
//        return;
//    }
//    m_destructing = true;
//    stop();
    if (m_thread.joinable()){
        m_thread.join();
    }
}
RunnablePanelWidget::~RunnablePanelWidget(){
    ProgramTracker::instance().remove_program(m_instance_id);
    RunnablePanelWidget::request_program_stop();
    join_program_thread();
}

std::string RunnablePanelWidget::stats(){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_current_stats){
        return m_current_stats->to_str();
    }
    return "";
}
WallClock RunnablePanelWidget::timestamp() const{
    return m_timestamp.load(std::memory_order_acquire);
}


bool RunnablePanelWidget::start(){
    bool ret = false;
    switch (state()){
    case ProgramState::NOT_READY:
        m_logger.log("Recevied Start Request: Program is not ready.", COLOR_RED);
        break;
    case ProgramState::STOPPED:{
        m_logger.log("Received Start Request");
        QString error = check_validity();
        if (!error.isEmpty()){
            QMessageBox box;
            box.critical(nullptr, "Error", error);
            ret = false;
            break;
        }
        if (m_thread.joinable()){
            m_thread.join();
        }

        m_timestamp.store(current_time(), std::memory_order_release);
        m_state.store(ProgramState::RUNNING, std::memory_order_release);
        m_thread = std::thread(
            run_with_catch,
            "RunnablePanelWidget::run_program()",
            [=]{
                static_cast<RunnablePanelInstance&>(m_instance).reset_state();
                run_program();
            }
        );
        ret = true;
        break;
    }
    case ProgramState::RUNNING:
//    case ProgramState::FINISHED:
        m_logger.log("Received Start Request: Program is already running.");
//        m_state.store(ProgramState::STOPPING, std::memory_order_release);
//        on_stop();
        ret = false;
        break;
    case ProgramState::STOPPING:
        m_logger.log("Received Start Request: Program is already stopping.");
        ret = false;
        break;
    }
    update_ui_after_program_state_change();
    return ret;
}
bool RunnablePanelWidget::request_program_stop(){
    if (state() != ProgramState::RUNNING){
        return false;
    }
    m_logger.log("Received Stop Request");
    m_state.store(ProgramState::STOPPING, std::memory_order_release);
    if (m_start_button == nullptr){
        return true;
    }
    m_start_button->setText("Stopping Program...");
    m_listener.on_busy(m_instance);
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_scope){
            m_scope->cancel(std::make_exception_ptr(ProgramCancelledException()));
        }
    }
    return true;
}



RunnablePanelWidget::RunnablePanelWidget(
    QWidget& parent,
    RunnablePanelInstance& instance,
    PanelListener& listener
)
    : PanelWidget(parent, instance, listener)
    , m_logger(listener.raw_logger(), "Program")
    , m_status_bar(nullptr)
    , m_start_button(nullptr)
    , m_timestamp(current_time())
    , m_state(ProgramState::NOT_READY)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_header_holder = new QVBoxLayout();
    layout->addLayout(m_header_holder);

    {
        QScrollArea* scroll_outer = new QScrollArea(this);
        layout->addWidget(scroll_outer);
        scroll_outer->setWidgetResizable(true);

        m_scroll_inner = new QWidget(scroll_outer);
        scroll_outer->setWidget(m_scroll_inner);
        QVBoxLayout* scroll_layout = new QVBoxLayout(m_scroll_inner);
        scroll_layout->setAlignment(Qt::AlignTop);

        m_body_holder = new QVBoxLayout();
        scroll_layout->addLayout(m_body_holder);

        m_options_holder = new QVBoxLayout();
        scroll_layout->addLayout(m_options_holder);
        scroll_layout->addStretch(1);
    }

    m_status_bar_holder = new QVBoxLayout();
    layout->addLayout(m_status_bar_holder);

    m_actions_holder = new QVBoxLayout();
    layout->addLayout(m_actions_holder);


    m_instance_id = ProgramTracker::instance().add_program(*this);
    connect(
        this, &RunnablePanelWidget::async_start,
        this, &RunnablePanelWidget::start
    );
    connect(
        this, &RunnablePanelWidget::async_stop,
        this, &RunnablePanelWidget::request_program_stop
    );
    connect(
        this, &RunnablePanelWidget::async_set_status,
        this, &RunnablePanelWidget::status_update
    );
}
void RunnablePanelWidget::construct(){
    m_header_holder->addWidget(m_header = make_header(*this));
    m_body = make_body(*m_scroll_inner);
    if (m_body){
        m_body_holder->addWidget(m_body);
    }
    m_options_holder->addWidget(m_options = make_options(*m_scroll_inner));
    m_status_bar_holder->addWidget(m_status_bar = make_status_bar(*this));
    m_actions_holder->addWidget(m_actions = make_actions(*this));
}
QWidget* RunnablePanelWidget::make_body(QWidget& parent){
    return nullptr;
}
BatchWidget* RunnablePanelWidget::make_options(QWidget& parent){
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    BatchWidget* options = static_cast<BatchWidget*>(instance.m_options.make_ui(parent));
    options->update_visibility();
    return options;
}
QLabel* RunnablePanelWidget::make_status_bar(QWidget& parent){
    QLabel* status_bar = new QLabel(&parent);
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
    action_layout->setContentsMargins(0, 0, 0, 0);

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

    update_ui_after_program_state_change();
    connect(
        this, &RunnablePanelWidget::signal_reset,
        this, [=]{ update_ui_after_program_state_change(); }
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
            switch (state()){
            case ProgramState::NOT_READY:
                break;
            case ProgramState::STOPPED:
                start();
                break;
            case ProgramState::RUNNING:
//            case ProgramState::FINISHED:
                request_program_stop();
                break;
            case ProgramState::STOPPING:
                break;
            }
//            update_ui();
        }
    );
    connect(
        m_default_button, &QPushButton::clicked,
        this, [=](bool){
            QMessageBox::StandardButton button = QMessageBox::question(
                nullptr,
                "Restore Defaults",
                "Are you sure you wish to restore settings back to defaults? This will wipe the current settings.",
                QMessageBox::Ok | QMessageBox::Cancel
            );
            if (button == QMessageBox::Ok){
                restore_defaults();
            }
        }
    );

    return actions_widget;
}


#if 0
void RunnablePanelWidget::redraw_options(){
    if (m_options != nullptr){
        m_options_holder->removeWidget(m_options);
        delete m_options;
        m_options = nullptr;
    }
    m_options_holder->addWidget(m_options = make_options(*m_scroll_inner));
}
#endif


QString RunnablePanelWidget::check_validity() const{
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    return instance.check_validity();
}
void RunnablePanelWidget::restore_defaults(){
    m_options->restore_defaults();
}
void RunnablePanelWidget::load_historical_stats(){
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    std::unique_ptr<StatsTracker> stats = instance.make_stats();
    if (stats){
        StatSet set;
        set.open_from_file(GlobalSettings::instance().STATS_FILE);
        const std::string& identifier = instance.descriptor().identifier();
        StatList& list = set[identifier];
        if (list.size() != 0){
            list.aggregate(*stats);
        }
        emit async_set_status(QString::fromStdString(stats->to_str()));
//        m_status_bar->setText(QString::fromStdString(stats->to_str()));
//        m_status_bar->setVisible(true);
    }

    std::lock_guard<std::mutex> lg(m_lock);
    m_historical_stats = std::move(stats);
}
void RunnablePanelWidget::update_historical_stats(){
    bool ok;
    {
//        std::lock_guard<std::mutex> lg(m_lock);
        if (!m_current_stats){
            return;
        }
        m_logger.log("Saving historical stats...");
        ok = StatSet::update_file(
            GlobalSettings::instance().STATS_FILE,
            m_instance.descriptor().identifier(),
            *m_current_stats
        );
    }
    if (ok){
        m_logger.log("Stats successfully saved!", COLOR_BLUE);
    }else{
        m_logger.log("Unable to save stats.", COLOR_RED);
#if 0
        QMetaObject::invokeMethod(
            this,
            "show_stats_warning",
            Qt::AutoConnection
        );
#endif
//        show_stats_warning();
    }
//    settings.stat_sets.open_from_file(settings.stats_file);
}

void RunnablePanelWidget::update_ui_after_program_state_change(){
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_start_button == nullptr){
        return;
    }
    m_start_button->setEnabled(state != ProgramState::STOPPING);
    switch (state){
    case ProgramState::NOT_READY:
        m_start_button->setText("Loading...");
        m_listener.on_busy(m_instance);
        break;
    case ProgramState::STOPPED:
        m_start_button->setText("Start Program...");
//        m_start_button->setEnabled(settings_valid());
        m_listener.on_idle(m_instance);
        break;
    case ProgramState::RUNNING:
        m_start_button->setText("Stop Program...");
        m_listener.on_busy(m_instance);
        break;
//    case ProgramState::FINISHED:
//        m_start_button->setText("Program Finished! Click to stop.");
//        m_listener.on_busy(m_instance);
//        break;
    case ProgramState::STOPPING:
        m_start_button->setText("Stopping Program...");
        m_listener.on_busy(m_instance);
        break;
    }

    bool enabled = state == ProgramState::STOPPED;
    m_default_button->setEnabled(enabled);
    m_options->setEnabled(enabled);
}


void RunnablePanelWidget::status_update(QString status){
    if (status.size() <= 0){
        m_status_bar->setVisible(false);
        m_status_bar->setText(status);
    }else{
        m_status_bar->setText(status);
        m_status_bar->setVisible(true);
    }
}




}
