/*  Multi-Console Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMessageBox>
#include "Common/Cpp/ScopeExit.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ResourceDownload/ProgramResourceDownloadWidget.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "MultiConsoleProgramWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<GameConsole::MultiConsoleProgramWidget>;

namespace GameConsole{


MultiConsoleProgramWidget::~MultiConsoleProgramWidget(){
    m_session.remove_listener(*this);
}
MultiConsoleProgramWidget::MultiConsoleProgramWidget(QWidget& parent, MultiConsoleProgramSession& session)
    : QWidget(&parent)
    , m_session(session)
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
}

void MultiConsoleProgramWidget::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [this, state]{
        if (state != ProgramState::STOPPED){
            m_session.system().lock_controllers("Program is Running");
        }else{
            m_session.system().unlock_controllers();
        }
        m_session.options().report_program_state(state != ProgramState::STOPPED);
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
void MultiConsoleProgramWidget::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }, Qt::QueuedConnection);
}
void MultiConsoleProgramWidget::error(const std::string& message){
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}


void MultiConsoleProgramWidget::download_error(const std::string& message){
    if (m_popup_is_open.exchange(true)){ // only show popups if one isn't already open
        return;
    }

    ScopeExit scope([&]{
        m_popup_is_open.store(false, std::memory_order_release);
    });

    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}
void MultiConsoleProgramWidget::download_added(std::shared_ptr<ResourceDownload> download_ptr){
    QMetaObject::invokeMethod(this, [this, download_ptr = std::move(download_ptr)]() mutable{
        this->ensure_downloads_table()->add_download(std::move(download_ptr));
    }, Qt::QueuedConnection);
}
void MultiConsoleProgramWidget::all_downloads_done(){
    QMetaObject::invokeMethod(this, [this]{
        this->ensure_downloads_table()->remove_all_downloads();
    }, Qt::QueuedConnection);
}
ProgramResourceDownloadTableWidget* MultiConsoleProgramWidget::ensure_downloads_table(){
    if (!m_internal_lazy_downloads_table){
        m_internal_lazy_downloads_table = new ProgramResourceDownloadTableWidget(*this);
        m_internal_lazy_downloads_table->setVisible(false);
        m_layout->addWidget(m_internal_lazy_downloads_table);
    }
    return m_internal_lazy_downloads_table;
}






}
}
