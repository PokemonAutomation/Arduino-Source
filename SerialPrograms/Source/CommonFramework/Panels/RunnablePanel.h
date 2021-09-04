/*  Runnable Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnablePanel_H
#define PokemonAutomation_RunnablePanel_H

#include <thread>
#include <QLabel>
#include <QPushButton>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Options/BatchOption.h"
#include "CommonFramework/Panels/Panel.h"

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
    using PanelInstance::PanelInstance;

    void add_option(ConfigOption& option, QString serialization_string){
        m_options.add_option(option, std::move(serialization_string));
    }

    const RunnablePanelDescriptor& descriptor() const{
        return static_cast<const RunnablePanelDescriptor&>(m_descriptor);
    }

    virtual bool is_valid() const;
    virtual void restore_defaults();

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

private:
    friend class RunnablePanelWidget;
    BatchOption m_options;
};



class RunnablePanelWidget : public PanelWidget{
    Q_OBJECT

public:
    virtual ~RunnablePanelWidget();

    ProgramState state() const{ return m_state.load(std::memory_order_acquire); }

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
    void construct();
    virtual QWidget* make_options(QWidget& parent);
    virtual QLabel* make_status_bar(QWidget& parent);
    virtual QWidget* make_actions(QWidget& parent);

protected:
    virtual bool settings_valid() const;
    virtual void restore_defaults();

    virtual void update_ui();
    void set_status(QString status);

    virtual void on_stop();

    virtual void run_program() = 0;


signals:    //  Protected Signals
    void signal_cancel();
    void signal_error(QString message);
    void signal_reset();

    void async_set_status(QString status);


protected:
    friend class RunnablePanelInstance;

    TaggedLogger m_logger;

    BatchOptionUI* m_options;
    QLabel* m_status_bar;

    QPushButton* m_start_button;
    QPushButton* m_default_button;

    std::atomic<ProgramState> m_state;
    std::thread m_thread;

    bool m_destructing = false;
};



}
#endif
