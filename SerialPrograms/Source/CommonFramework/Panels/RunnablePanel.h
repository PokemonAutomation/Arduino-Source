/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnablePanel_H
#define PokemonAutomation_RunnablePanel_H

#include <thread>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption/BatchOption.h"
#include "CommonFramework/Options/BatchOption/BatchWidget.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Panels/Panel.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"

namespace PokemonAutomation{


#if 0
class RunnableProgramDescriptor : public PanelDescriptor{
public:
    RunnableProgramDescriptor(
        std::string identifier,
        QString display_name,
        QString doc_link,
        QString description
    );
};
#endif
using RunnablePanelDescriptor = PanelDescriptor;



class RunnablePanelInstance : public PanelInstance{
public:
    RunnablePanelInstance(const PanelDescriptor& descriptor);

    void add_option(ConfigOption& option, QString serialization_string){
        m_options.add_option(option, std::move(serialization_string));
    }

    const RunnablePanelDescriptor& descriptor() const{
        return static_cast<const RunnablePanelDescriptor&>(m_descriptor);
    }

    virtual std::unique_ptr<StatsTracker> make_stats() const{ return nullptr; }

    virtual QString check_validity() const;
    virtual void restore_defaults();
    virtual void reset_state();

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

private:
    friend class RunnablePanelWidget;
    BatchOption m_options;
protected:
    EventNotificationOption NOTIFICATION_ERROR_RECOVERABLE;
    EventNotificationOption NOTIFICATION_ERROR_FATAL;
};



class RunnablePanelWidget : public PanelWidget{
    Q_OBJECT

public:
    virtual ~RunnablePanelWidget();

    ProgramState state() const{ return m_state.load(std::memory_order_acquire); }
    std::string stats();
    std::chrono::system_clock::time_point timestamp() const;

//    //  Reset serial if possible.
//    bool reset_serial();    //  Must call on main thread.

    //  Start program if possible.
    bool start();           //  Must call on main thread.

    //  Stops the program if it is running.
    bool stop();            //  Must call on main thread.

signals:    //  Public Signals
    void async_start();
    void async_stop();


protected:
    //  Call this in the destructor of all child classes.
    void on_destruct_stop();

    RunnablePanelWidget(
        QWidget& parent,
        RunnablePanelInstance& instance,
        PanelListener& listener
    );

    //  Call immediately after construction.
    void construct();

    //  Child classes can override these with their own customizations.
    virtual QWidget* make_body(QWidget& parent);
    virtual BatchWidget* make_options(QWidget& parent);
    virtual QLabel* make_status_bar(QWidget& parent);
    virtual QWidget* make_actions(QWidget& parent);

//    void redraw_options();

protected:
    virtual QString check_validity() const;
    virtual void restore_defaults();

    void load_historical_stats();
    void update_historical_stats();

    virtual void update_ui_after_program_state_change();
    void set_status(QString status);

    virtual void on_stop();

    virtual void run_program() = 0;

protected:

signals:    //  Protected Signals
    void signal_cancel();
    void signal_error(QString message);
    void signal_reset();

    void async_set_status(QString status);


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
    QLabel* m_status_bar = nullptr;
    QWidget* m_actions = nullptr;

    QPushButton* m_start_button;
    QPushButton* m_default_button;

    std::atomic<std::chrono::system_clock::time_point> m_timestamp;
    std::atomic<ProgramState> m_state;
    std::thread m_thread;

    bool m_destructing = false;

    std::mutex m_lock;
    std::unique_ptr<StatsTracker> m_historical_stats;
    std::unique_ptr<StatsTracker> m_current_stats;
};



}
#endif
