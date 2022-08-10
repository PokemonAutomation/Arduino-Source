/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnablePanelWidget_H
#define PokemonAutomation_RunnablePanelWidget_H

#include <mutex>
#include <thread>
#include "CommonFramework/Globals.h"
#include "Integrations/ProgramTrackerInterfaces.h"
#include "PanelWidget.h"
#include "RunnablePanel.h"

class QBoxLayout;
class QLabel;
class QPushButton;

namespace PokemonAutomation{

class BatchWidget;
class CancellableScope;
class StatsBar;
class RunnablePanelActionBar;


class RunnablePanelWidget : public PanelWidget, public TrackableProgram{
    Q_OBJECT

public:
    virtual ~RunnablePanelWidget();

    virtual const std::string& identifier() const override final{ return m_instance.descriptor().identifier(); }
    virtual ProgramState current_state() const override final{ return m_state.load(std::memory_order_acquire); }
    virtual std::string current_stats() const override final;
    virtual WallClock timestamp() const override final;

//    //  Reset serial if possible.
//    bool reset_serial();    //  Must call on main thread.

    //  Start program if possible.
    bool start();           //  Must call on main thread.

    //  Stops the program if it is running.
//    bool stop();            //  Must call on main thread.
    virtual bool request_program_stop();    //  Must call on main thread.

    virtual void async_start() override final;
    virtual void async_stop() override final;


protected:
    //  Call this in the destructor of all child classes.
    void join_program_thread();

    RunnablePanelWidget(
        QWidget& parent,
        RunnablePanelInstance& instance,
        PanelHolder& holder
    );

    //  Call immediately after construction.
    void construct();

    //  Child classes can override these with their own customizations.
    virtual QWidget* make_body(QWidget& parent);
    virtual BatchWidget* make_options(QWidget& parent);
    StatsBar* make_status_bar(QWidget& parent);
    //  Make action widget, which is a UI area with button "Start Program!" and "Restore Defaults".
    RunnablePanelActionBar* make_actions(QWidget& parent);

//    void redraw_options();

protected:
    virtual std::string check_validity() const;
    virtual void restore_defaults();

    void load_historical_stats();
    void update_historical_stats();

    virtual void update_ui_after_program_state_change();
    void status_update(std::string status);

    virtual void run_program() = 0;

protected:

signals:    //  Protected Signals
    void signal_cancel();
    void signal_error(std::string message);
    void signal_reset();

    void async_set_status(std::string status);


protected:
    friend class RunnablePanelInstance;

    uint64_t m_instance_id = 0;
    TaggedLogger m_logger;

    QBoxLayout* m_header_holder;
    QBoxLayout* m_body_holder;
    QBoxLayout* m_options_holder;
    QBoxLayout* m_status_bar_holder;
    QBoxLayout* m_actions_holder;

    QWidget* m_scroll_inner;

    QWidget* m_header = nullptr;
    QWidget* m_body = nullptr;
    BatchWidget* m_options = nullptr;
    StatsBar* m_status_bar = nullptr;
    RunnablePanelActionBar* m_actions = nullptr;

//    QPushButton* m_start_button;
//    QPushButton* m_default_button;

    std::atomic<WallClock> m_timestamp;
    std::atomic<ProgramState> m_state;
    std::thread m_thread;

//    bool m_destructing = false;

    mutable std::mutex m_lock;
//    std::string m_historical_stats_str;
    std::unique_ptr<StatsTracker> m_historical_stats;
    std::unique_ptr<StatsTracker> m_current_stats;
    CancellableScope* m_scope = nullptr;
};



}
#endif
