/*  RunnableProgram
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnableProgram_H
#define PokemonAutomation_RunnableProgram_H

#include <thread>
#include <QLabel>
#include <QPushButton>
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Options/ConfigOption.h"
#include "CommonFramework/Panels/RightPanel.h"
#include "SwitchSetup.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


using PABotBase = PokemonAutomation::PABotBase;
using BotBase = BotBase;




class RunnableProgram : public RightPanel{
public:
    RunnableProgram(
        FeedbackType feedback,
        PABotBaseLevel min_pabotbase_level,
        QString name,
        QString doc_link,
        QString description
    );
    void from_json(const QJsonValue& json);
    virtual QJsonValue to_json() const override;

    bool is_valid() const;
    void restore_defaults();

    virtual std::unique_ptr<StatsTracker> make_stats() const{ return nullptr; }
    virtual QWidget* make_ui(MainWindow& window) override;

protected:
    friend class RunnableProgramUI;
    const FeedbackType m_feedback;
    const PABotBaseLevel m_min_pabotbase_level;
    SwitchSetupFactory* m_setup;
    std::vector<std::pair<ConfigOption*, QString>> m_options;
};


class RunnableProgramUI : public RightPanelUI{
    Q_OBJECT
    friend class RunnableProgram;

protected:
    RunnableProgramUI(RunnableProgram& factory, MainWindow& parent);
    virtual void append_description(QWidget& parent, QVBoxLayout& layout) override;
    virtual void make_body(QWidget& parent, QVBoxLayout& layout) override;

public:
    virtual ~RunnableProgramUI();
    void stop();

    virtual bool settings_valid() const;
    void restore_defaults();
    virtual ProgramState update_ui();

    void set_status(QString status);

private:
    void on_stop();
    void reset_connections();
    void update_historical_stats();
    virtual void program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ){}

    void run_program();

signals:
    void signal_cancel();
    void signal_error(QString message);
    void signal_reset();

public slots:
    void show_stats_warning() const;

protected:
    static BotBase& sanitize_botbase(BotBase* botbase);

protected:
    const QString& m_name;
    MainWindow& m_window;
    TaggedLogger m_logger;

    SwitchSetup* m_setup;
    std::vector<ConfigOptionUI*> m_options;

    QLabel* m_status_bar;
    QPushButton* m_start_button;
    QPushButton* m_default_button;

    std::unique_ptr<StatsTracker> m_stats;

//    ProgramEnvironment m_environment;
    std::atomic<ProgramState> m_state;
    std::thread m_thread;
};



}
}
#endif





