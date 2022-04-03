/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H
#define PokemonAutomation_NintendoSwitch_MultiSwitchProgram_H

#include <functional>
#include "Common/Cpp/FixedLimitVector.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "NintendoSwitch_MultiSwitchSystem.h"
#include "NintendoSwitch_RunnableProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchProgramEnvironment : public ProgramEnvironment{
public:
    ~MultiSwitchProgramEnvironment();

    FixedLimitVector<ConsoleHandle> consoles;

    //  Run the specified lambda for all switches in parallel.
    void run_in_parallel(
        CancellableScope& scope,
        const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
    );

    //  Run the specified lambda for switch indices [s, e) in parallel.
    void run_in_parallel(
        CancellableScope& scope, size_t s, size_t e,
        const std::function<void(ConsoleHandle& console, BotBaseContext& context)>& func
    );

private:
    friend class MultiSwitchProgramWidget;
    friend class MultiSwitchProgramUI;
    MultiSwitchProgramEnvironment(
        ProgramInfo program_info,
        LoggerQt& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        FixedLimitVector<ConsoleHandle> p_switches
    );
};



class MultiSwitchProgramDescriptor : public RunnableSwitchProgramDescriptor{
public:
    MultiSwitchProgramDescriptor(
        std::string identifier,
        QString category, QString display_name,
        QString doc_link,
        QString description,
        FeedbackType feedback, bool allow_commands_while_running,
        PABotBaseLevel min_pabotbase_level,
        size_t min_switches,
        size_t max_switches,
        size_t default_switches
    );

    size_t min_switches() const{ return m_min_switches; }
    size_t max_switches() const{ return m_max_switches; }
    size_t default_switches() const{ return m_default_switches; }

private:
    const size_t m_min_switches;
    const size_t m_max_switches;
    const size_t m_default_switches;
};



class MultiSwitchProgramInstance : public RunnableSwitchProgramInstance{
public:
    MultiSwitchProgramInstance(const MultiSwitchProgramDescriptor& descriptor);

    //  Called when the user changes the # of Switches.
    virtual void update_active_consoles(){}

    size_t system_count() const{ return m_switches.count(); }

    virtual QWidget* make_widget(QWidget& parent, PanelListener& listener) override;
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) = 0;

private:
    friend class MultiSwitchProgramWidget;

    MultiSwitchSystemFactory m_switches;
};





}
}
#endif

