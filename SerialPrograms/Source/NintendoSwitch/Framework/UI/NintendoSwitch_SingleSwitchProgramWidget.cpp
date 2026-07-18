/*  Single Switch Program Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include "Common/Qt/ShutdownWithEvents.h"
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "CommonFramework/Startup/NewVersionCheck.h"
#include "CommonFramework/Panels/PanelTools.h"
#include "CommonFramework/Panels/UI/PanelElements.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ResourceDownload/ProgramResourceDownloadWidget.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgramOption.h"
#include "NintendoSwitch_SingleSwitchProgramWidget.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



SingleSwitchProgramWidget2::~SingleSwitchProgramWidget2(){
    m_session.remove_listener(*this);
    delete m_actions_bar;
    delete m_stats_bar;
    delete m_options;
    delete m_system;

    shutdown_with_events(
        m_session.logger(),
        "SingleSwitchProgramWidget2",
        [this]{ return m_session.try_shutdown(); }
    );
}
SingleSwitchProgramWidget2::SingleSwitchProgramWidget2(
    QWidget& parent,
    SingleSwitchProgramOption& option,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_holder(holder)
    , m_session(option, 0)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    const SingleSwitchProgramDescriptor& descriptor = option.descriptor();

    CollapsibleGroupBox* header = make_panel_header(
        *this,
        descriptor.display_name(),
        descriptor.doc_link(),
        descriptor.description(),
        descriptor.color_class()
    );
    m_layout->addWidget(header);

    if (descriptor.deprecated()){
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

        m_system = new SwitchSystemWidget(
            *this,
            m_session.system(),
            m_session.instance_id()
        );
        scroll_layout->addWidget(m_system);

        m_options = ConfigWidget::make_from_option(option.options(), this);
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
            option.restore_defaults();
            m_options->update_all(false);
        }
    );

    m_session.add_listener(*this);
}

void SingleSwitchProgramWidget2::state_change(ProgramState state){
    QMetaObject::invokeMethod(this, [this, state]{
        m_system->update_ui(state);
        m_options->option().report_program_state(state != ProgramState::STOPPED);
//        cout << "state = " << (state != ProgramState::STOPPED) << endl;
//        if (m_option.descriptor().lock_options_while_running()){
//            m_options->widget().setEnabled(state == ProgramState::STOPPED);
//        }
        m_actions_bar->set_state(state);
        if (state == ProgramState::STOPPED){
            m_holder.on_idle();
            check_new_version();
        }else{
            m_holder.on_busy();
        }

        if(state == ProgramState::STOPPING){
            ensure_downloads_table()->remove_all_downloads();
        }        
    }, Qt::QueuedConnection);
}
void SingleSwitchProgramWidget2::stats_update(const StatsTracker* current_stats, const StatsTracker* historical_stats){
    QMetaObject::invokeMethod(this, [this, current_stats, historical_stats]{
        m_stats_bar->set_stats(
            current_stats == nullptr ? "" : current_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN),
            historical_stats == nullptr ? "" : historical_stats->to_str(StatsTracker::DISPLAY_ON_SCREEN)
        );
    }, Qt::QueuedConnection);
}
void SingleSwitchProgramWidget2::error(const std::string& message){
    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
}
void SingleSwitchProgramWidget2::download_error(const std::string& message){
    if (m_popup_is_open.exchange(true)){ // only show popups if one isn't already open
        return;
    }

    QMetaObject::invokeMethod(this, [message]{
        QMessageBox box;
        box.critical(nullptr, "Error", QString::fromStdString(message));
    }, Qt::QueuedConnection);
    m_popup_is_open.store(false);
}

void SingleSwitchProgramWidget2::download_added(std::shared_ptr<ResourceDownload> download_ptr){
    QMetaObject::invokeMethod(this, [this, download_ptr = std::move(download_ptr)]() mutable{
        this->ensure_downloads_table()->add_download(std::move(download_ptr));
    }, Qt::QueuedConnection);
}

void SingleSwitchProgramWidget2::all_downloads_done(){
    QMetaObject::invokeMethod(this, [this]{
        this->ensure_downloads_table()->remove_all_downloads();
    }, Qt::QueuedConnection);
}


ProgramResourceDownloadTableWidget* SingleSwitchProgramWidget2::ensure_downloads_table() {
    if (!m_internal_lazy_downloads_table) {
        m_internal_lazy_downloads_table = new ProgramResourceDownloadTableWidget(*this);
        m_internal_lazy_downloads_table->setVisible(false);
        m_layout->addWidget(m_internal_lazy_downloads_table);
    }
    return m_internal_lazy_downloads_table;
}






}
}
