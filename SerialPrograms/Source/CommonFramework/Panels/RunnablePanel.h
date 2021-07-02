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
#include "CommonFramework/Windows/OutputWindow.h"
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

    const RunnablePanelDescriptor& descriptor() const{
        return static_cast<const RunnablePanelDescriptor&>(m_descriptor);
    }

    bool is_valid() const;
    void restore_defaults();

public:
    //  Serialization
    virtual void from_json(const QJsonValue& json) override;
    virtual QJsonValue to_json() const override;

protected:
    friend class RunnablePanelWidget;

    std::vector<std::pair<ConfigOption*, QString>> m_options;
};



class RunnablePanelWidget : public PanelWidget{
    Q_OBJECT

public:
    virtual ~RunnablePanelWidget();
protected:
    void stop();

protected:
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




signals:
    void signal_cancel();
    void signal_error(QString message);
    void signal_reset();

protected:
    friend class RunnablePanelInstance;

    TaggedLogger m_logger;

    std::vector<ConfigOptionUI*> m_options;
    QLabel* m_status_bar;

    QPushButton* m_start_button;
    QPushButton* m_default_button;

    std::atomic<ProgramState> m_state;
    std::thread m_thread;

    bool m_destructing = false;
};



}
#endif
