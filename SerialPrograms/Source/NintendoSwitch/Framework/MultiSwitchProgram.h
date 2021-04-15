/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MultiSwitchProgram_H
#define PokemonAutomation_MultiSwitchProgram_H

#include "Common/Clientside/AsyncDispatcher.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "MultiSwitchSystem.h"
#include "RunnableSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchProgramEnvironment : public ProgramEnvironment{
public:
    AsyncDispatcher dispatcher;
    std::vector<ConsoleHandle> consoles;

    //  Run the specified lambda for all switches in parallel.
    void run_in_parallel(
        const std::function<void(ConsoleHandle& console)>& func
    );

    //  Run the specified lambda for switch indices [s, e) in parallel.
    void run_in_parallel(
        size_t s, size_t e,
        const std::function<void(ConsoleHandle& console)>& func
    );

private:
    friend class MultiSwitchProgramUI;
    MultiSwitchProgramEnvironment(
        Logger& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        std::vector<ConsoleHandle> p_switches
    );
};


class MultiSwitchProgram : public RunnableProgram{
public:
    MultiSwitchProgram(
        FeedbackType feedback,
        PABotBaseLevel min_pabotbase,
        QString name,
        QString doc_link,
        QString description,
        size_t min_switches,
        size_t max_switches,
        size_t switches
    );

    size_t count() const{ return m_switches.count(); }
    virtual void program(MultiSwitchProgramEnvironment& env) const = 0;

private:
    MultiSwitchSystemFactory m_switches;
};


class MultiSwitchProgramUI final : public RunnableProgramUI{
public:
    MultiSwitchProgramUI(MultiSwitchProgram& factory, MainWindow& window);
    ~MultiSwitchProgramUI();

    virtual void program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ) override;
};


template <typename Program>
class MultiSwitchProgramWrapper final : public Program{
public:
    MultiSwitchProgramWrapper() = default;
    MultiSwitchProgramWrapper(const QJsonValue& json)
        : Program()
    {
        this->from_json(json);
    }
    virtual QWidget* make_ui(MainWindow& window) override{
        return new MultiSwitchProgramUI(*this, window);
    }
};



}
}
#endif

