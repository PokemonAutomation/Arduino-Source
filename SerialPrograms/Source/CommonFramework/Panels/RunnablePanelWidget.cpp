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
#include "PanelElements.h"
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
        std::string error = check_validity();
        if (!error.empty()){
            QMessageBox box;
            box.critical(nullptr, "Error", QString::fromStdString(error));
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
    if (m_actions == nullptr){
        return true;
    }
    m_actions->set_state(ProgramState::STOPPING);
    m_holder.on_busy(m_instance);
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
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
    , m_logger(holder.raw_logger(), "Program")
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



    connect(
        this, &RunnablePanelWidget::signal_reset,
        this, [=]{ update_ui_after_program_state_change(); }
    );
    connect(
        this, &RunnablePanelWidget::signal_error,
        this, [](std::string message){
            QMessageBox box;
            box.critical(nullptr, "Error", QString::fromStdString(message));
        }
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
StatsBar* RunnablePanelWidget::make_status_bar(QWidget& parent){
    return new StatsBar(parent);
}
RunnablePanelActionBar* RunnablePanelWidget::make_actions(QWidget& parent){
    ProgramState state = m_state.load(std::memory_order_acquire);

    RunnablePanelActionBar* ret = new RunnablePanelActionBar(parent, state);

    connect(
        ret, &RunnablePanelActionBar::start_clicked,
        this, [=](){
            switch (this->state()){
            case ProgramState::NOT_READY:
                break;
            case ProgramState::STOPPED:
                start();
                break;
            case ProgramState::RUNNING:
                request_program_stop();
                break;
            case ProgramState::STOPPING:
                break;
            }
//            update_ui();
        }
    );
    connect(
        ret, &RunnablePanelActionBar::defaults_clicked,
        this, [=](){
            restore_defaults();
        }
    );



    return ret;
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


std::string RunnablePanelWidget::check_validity() const{
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    return instance.check_validity();
}
void RunnablePanelWidget::restore_defaults(){
    m_options->restore_defaults();
}
void RunnablePanelWidget::load_historical_stats(){
    RunnablePanelInstance& instance = static_cast<RunnablePanelInstance&>(m_instance);
    std::unique_ptr<StatsTracker> stats = instance.descriptor().make_stats();
    std::string stats_str;
    if (stats){
        StatSet set;
        set.open_from_file(GlobalSettings::instance().STATS_FILE);
        const std::string& identifier = instance.descriptor().identifier();
        StatList& list = set[identifier];
        if (list.size() != 0){
            list.aggregate(*stats);
        }
        stats_str = stats->to_str();
        emit async_set_status(stats_str);
//        m_status_bar->setText(QString::fromStdString(stats->to_str()));
//        m_status_bar->setVisible(true);
    }

    std::lock_guard<std::mutex> lg(m_lock);
//    m_historical_stats_str = std::move(stats_str);
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
    m_logger.log("Updating UI after program state change...");
    ProgramState state = m_state.load(std::memory_order_acquire);
    if (m_actions == nullptr){
        return;
    }
    m_actions->set_state(state);
    switch (state){
    case ProgramState::NOT_READY:
        m_logger.log("Updating UI after program state change... Program not ready.");
        m_holder.on_busy(m_instance);
        break;
    case ProgramState::STOPPED:
        m_logger.log("Updating UI after program state change... Program stopped.");
//        m_start_button->setEnabled(settings_valid());
        m_holder.on_idle(m_instance);
        break;
    case ProgramState::RUNNING:
        m_logger.log("Updating UI after program state change... Program running.");
        m_holder.on_busy(m_instance);
        break;
//    case ProgramState::FINISHED:
//        m_logger.log("Updating UI after program state change... Program finished.");
//        m_start_button->setText("Program Finished! Click to stop.");
//        m_listener.on_busy(m_instance);
//        break;
    case ProgramState::STOPPING:
        m_logger.log("Updating UI after program state change... Program stopping.");
        m_holder.on_busy(m_instance);
        break;
    }

    bool enabled = state == ProgramState::STOPPED;
    m_options->setEnabled(enabled);
}


void RunnablePanelWidget::status_update(std::string status){
    if (status.empty()){
        m_status_bar->setVisible(false);
        m_status_bar->setText("");
    }else{
        m_status_bar->setText(QString::fromStdString(status));
        m_status_bar->setVisible(true);
    }
}




}
