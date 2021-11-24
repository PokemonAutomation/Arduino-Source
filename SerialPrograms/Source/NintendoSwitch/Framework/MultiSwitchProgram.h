/*  Multi-Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_MultiSwitchProgram_H
#define PokemonAutomation_MultiSwitchProgram_H

#include "Common/Cpp/FixedLimitVector.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "MultiSwitchSystem.h"
#include "RunnableSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class MultiSwitchProgramEnvironment : public ProgramEnvironment{
public:
    FixedLimitVector<ConsoleHandle> consoles;

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
    friend class MultiSwitchProgramWidget;
    friend class MultiSwitchProgramUI;
    MultiSwitchProgramEnvironment(
        ProgramInfo program_info,
        Logger& logger,
        StatsTracker* current_stats,
        const StatsTracker* historical_stats,
        FixedLimitVector<ConsoleHandle> p_switches
    );
};



class MultiSwitchProgramDescriptor : public RunnableSwitchProgramDescriptor{
public:
    MultiSwitchProgramDescriptor(
        std::string identifier,
        QString display_name,
        QString doc_link,
        QString description,
        FeedbackType feedback,
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
    virtual void program(MultiSwitchProgramEnvironment& env) = 0;

private:
    friend class MultiSwitchProgramWidget;

    MultiSwitchSystemFactory m_switches;
};



class MultiSwitchProgramWidget final : public RunnableSwitchProgramWidget{
public:
    static MultiSwitchProgramWidget* make(
        QWidget& parent,
        MultiSwitchProgramInstance& instance,
        PanelListener& listener
    );

    size_t system_count() const{
        return static_cast<MultiSwitchSystem&>(*m_setup).switch_count();
    }
    SwitchSystem& system(size_t index){
        return static_cast<MultiSwitchSystem&>(*m_setup)[index];
    }

private:
    using RunnableSwitchProgramWidget::RunnableSwitchProgramWidget;
    virtual ~MultiSwitchProgramWidget();

private:
    virtual void run_program(
        StatsTracker* current_stats,
        const StatsTracker* historical_stats
    ) override;

private:
    friend class MultiSwitchProgramInstance;
};





}
}
#endif

