/*  Computer Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QMessageBox>
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ResourceDownload/ProgramResourceDownloadWidget.h"
#include "ComputerProgramWidget.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<ComputerProgramWidget>;


ComputerProgramWidget::~ComputerProgramWidget(){
    m_session.remove_listener(*this);
}
ComputerProgramWidget::ComputerProgramWidget(
    QWidget& parent,
    ComputerProgramSession& session
)
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
        nullptr,
        session.options(),
        {m_stats_bar, m_actions_bar}
    );

    m_layout = static_cast<QVBoxLayout*>(this->layout());

    m_session.add_listener(*this);
}

void ComputerProgramWidget::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [this, state]{
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
void ComputerProgramWidget::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }, Qt::QueuedConnection);
}
void ComputerProgramWidget::error(const std::string& message){
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}

void ComputerProgramWidget::download_error(const std::string& message){
    if (m_popup_is_open.exchange(true)){ // only show popups if one isn't already open
        return;
    }
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
    m_popup_is_open.store(false);
}

void ComputerProgramWidget::download_added(std::shared_ptr<ResourceDownload> download_ptr){
    QMetaObject::invokeMethod(this, [this, download_ptr = std::move(download_ptr)]() mutable{
        this->ensure_downloads_table()->add_download(std::move(download_ptr));
    }, Qt::QueuedConnection);
}

void ComputerProgramWidget::all_downloads_done(){
    QMetaObject::invokeMethod(this, [this]{
        this->ensure_downloads_table()->remove_all_downloads();
    }, Qt::QueuedConnection);
}

ProgramResourceDownloadTableWidget* ComputerProgramWidget::ensure_downloads_table() {
    if (!m_internal_lazy_downloads_table) {
        m_internal_lazy_downloads_table = new ProgramResourceDownloadTableWidget(*this);
        m_internal_lazy_downloads_table->setVisible(false);
        m_layout->addWidget(m_internal_lazy_downloads_table);
    }
    return m_internal_lazy_downloads_table;
}






}
