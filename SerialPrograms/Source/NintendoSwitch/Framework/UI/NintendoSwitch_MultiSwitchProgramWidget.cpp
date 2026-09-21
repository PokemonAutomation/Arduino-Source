/*  Multi-Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QMessageBox>
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ResourceDownload/ProgramResourceDownloadWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_MultiSwitchProgramSession.h"
#include "NintendoSwitch_MultiSwitchProgramWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<NintendoSwitch::MultiSwitchProgramWidget2>;

namespace NintendoSwitch{



MultiSwitchProgramWidget2::~MultiSwitchProgramWidget2(){
    auto ScopeCheck = m_sanitizer.check_scope();
    m_session.system().remove_console_count_lock(*this);
    m_session.ProgramSession::remove_listener(*this);
    m_session.remove_listener(*this);
}


MultiSwitchProgramWidget2::MultiSwitchProgramWidget2(
    QWidget& parent,
    MultiSwitchProgramSession& session
)
    : QWidget(&parent)
    , m_session(session)
    , m_sanitizer("MultiSwitchProgramWidget2")
{
    m_stats_bar = new StatsBar(*this, m_session);
    m_actions_bar = new RunnablePanelActionBar(
        *this,
        session,
        session,
        m_session.current_state()
    );

    populate_panel_widget(
        *this,
        session.descriptor(),
        &m_session.system(),
        session.options(),
        {m_stats_bar, m_actions_bar}
    );

    m_layout = static_cast<QVBoxLayout*>(this->layout());

    m_session.add_listener(*this);
    m_session.ProgramSession::add_listener(*this);
}




void MultiSwitchProgramWidget2::state_change(ProgramState state){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, state]{

        if (state != ProgramState::STOPPED){
            m_session.system().add_console_count_lock(*this);
            m_session.system().lock_controllers("Program is Running");
        }else{
            m_session.system().unlock_controllers();
            m_session.system().remove_console_count_lock(*this);
        }
        m_session.options().report_program_state(state != ProgramState::STOPPED);

//        cout << "state = " << (state != ProgramState::STOPPED) << endl;
//        if (m_option.descriptor().lock_options_while_running()){
//            m_options->widget().setEnabled(state == ProgramState::STOPPED);
//        }
        m_actions_bar->set_state(state);
        if (state == ProgramState::STOPPED){
            global_panel_holder()->on_idle();
        }else{
            global_panel_holder()->on_busy();
        }

        if(state == ProgramState::STOPPING){
            ensure_downloads_table()->remove_all_downloads();
        }
    }, Qt::QueuedConnection);
}
void MultiSwitchProgramWidget2::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }, Qt::QueuedConnection);
}
void MultiSwitchProgramWidget2::error(const std::string& message){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}

void MultiSwitchProgramWidget2::download_error(const std::string& message){
    if (m_popup_is_open.exchange(true)){ // only show popups if one isn't already open
        return;
    }
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
    m_popup_is_open.store(false);
}

void MultiSwitchProgramWidget2::download_added(std::shared_ptr<ResourceDownload> download_ptr){
    QMetaObject::invokeMethod(this, [this, download_ptr = std::move(download_ptr)]() mutable{
        this->ensure_downloads_table()->add_download(std::move(download_ptr));
    }, Qt::QueuedConnection);
}

void MultiSwitchProgramWidget2::all_downloads_done(){
    QMetaObject::invokeMethod(this, [this]{
        this->ensure_downloads_table()->remove_all_downloads();
    }, Qt::QueuedConnection);
}


void MultiSwitchProgramWidget2::redraw_options(){
    auto ScopeCheck = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        m_session.options().report_program_state(false);
    }, Qt::QueuedConnection);
}


ProgramResourceDownloadTableWidget* MultiSwitchProgramWidget2::ensure_downloads_table() {
    if (!m_internal_lazy_downloads_table) {
        m_internal_lazy_downloads_table = new ProgramResourceDownloadTableWidget(*this);
        m_internal_lazy_downloads_table->setVisible(false);
        m_layout->addWidget(m_internal_lazy_downloads_table);
    }
    return m_internal_lazy_downloads_table;
}





}
}
