/*  Console Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMessageBox>
#include <QScrollArea>
#include "Common/Cpp/ScopeExit.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ResourceDownload/ProgramResourceDownloadWidget.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "ConsoleProgramWidget.h"

namespace PokemonAutomation{

template class RegisterUiStateQtWidget<GameConsole::ConsoleProgramWidget>;

namespace GameConsole{


ConsoleProgramWidget::~ConsoleProgramWidget(){
    m_session.remove_listener(*this);
}
ConsoleProgramWidget::ConsoleProgramWidget(QWidget& parent, ConsoleProgramSession& session)
    : QWidget(&parent)
    , m_session(session)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    const ConsoleProgramDescriptor& descriptor = session.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description(),
        descriptor.controller_class()
    );
    m_layout->addWidget(header);

    if (descriptor.deprecation() == PanelDeprecation::DEPRECATED){
        QMessageBox box;
        box.warning(
            nullptr,
            "Deprecation Notice",
            QString::fromStdString(
                "The program \"" + descriptor.display_name() + "\" is deprecated "
                "and no longer maintained. Please consider using a newer alternative."
            )
        );
    }


    {
        QScrollArea* scroll_outer = new QScrollArea(this);
        m_layout->addWidget(scroll_outer);
        scroll_outer->setWidgetResizable(true);

        QWidget* scroll_inner = new QWidget(scroll_outer);
        scroll_outer->setWidget(scroll_inner);
        QVBoxLayout* scroll_layout = new QVBoxLayout(scroll_inner);
        scroll_layout->setAlignment(Qt::AlignTop);

        UiWrapper wrapper = m_session.system().make_ui_component(this);
        scroll_layout->addWidget(dynamic_cast<QWidget*>(wrapper.release()));

        m_options = ConfigWidget::make_from_option(session.options(), this);
        scroll_layout->addWidget(&m_options->widget());

        scroll_layout->addStretch(1);
    }

    m_stats_bar = new StatsBar(*this);
    m_stats_bar->set_stats("", m_session.historical_stats());
    m_layout->addWidget(m_stats_bar);

    m_actions_bar = new RunnablePanelActionBar(*this, m_session.current_state());
    m_layout->addWidget(m_actions_bar);


    connect(
        m_actions_bar, &RunnablePanelActionBar::start_clicked,
        this, [&](ProgramState state){
            std::string error;
            switch (state){
            case ProgramState::STOPPED:
                error = m_session.start_program();
                break;
            case ProgramState::RUNNING:
                error = m_session.stop_program();
                break;
            default:;
            }
            if (!error.empty()){
                this->error(error);
            }
        }
    );
    connect(
        m_actions_bar, &RunnablePanelActionBar::defaults_clicked,
        this, [&]{
            std::lock_guard<Mutex> lg(m_session.program_lock());
            session.restore_defaults();
            m_options->update_all(false);
        }
    );

    m_session.add_listener(*this);
}

void ConsoleProgramWidget::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [this, state]{
        if (state != ProgramState::STOPPED){
            m_session.system().lock_controllers("Program is Running");
        }else{
            m_session.system().unlock_controllers();
        }
        m_options->option().report_program_state(state != ProgramState::STOPPED);
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
void ConsoleProgramWidget::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }, Qt::QueuedConnection);
}
void ConsoleProgramWidget::error(const std::string& message){
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}


void ConsoleProgramWidget::download_error(const std::string& message){
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
void ConsoleProgramWidget::download_added(std::shared_ptr<ResourceDownload> download_ptr){
    QMetaObject::invokeMethod(this, [this, download_ptr = std::move(download_ptr)]() mutable{
        this->ensure_downloads_table()->add_download(std::move(download_ptr));
    }, Qt::QueuedConnection);
}
void ConsoleProgramWidget::all_downloads_done(){
    QMetaObject::invokeMethod(this, [this]{
        this->ensure_downloads_table()->remove_all_downloads();
    }, Qt::QueuedConnection);
}
ProgramResourceDownloadTableWidget* ConsoleProgramWidget::ensure_downloads_table(){
    if (!m_internal_lazy_downloads_table){
        m_internal_lazy_downloads_table = new ProgramResourceDownloadTableWidget(*this);
        m_internal_lazy_downloads_table->setVisible(false);
        m_layout->addWidget(m_internal_lazy_downloads_table);
    }
    return m_internal_lazy_downloads_table;
}






}
}
