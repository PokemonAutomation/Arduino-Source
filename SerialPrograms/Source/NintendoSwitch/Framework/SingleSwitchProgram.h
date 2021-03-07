/*  Single Switch Program Template
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SingleSwitchProgram_H
#define PokemonAutomation_SingleSwitchProgram_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "SwitchSystem.h"
#include "RunnableSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SingleSwitchProgramEnvironment : public ProgramEnvironment{
public:
    Logger& logger;
    ConsoleHandle console;

private:
    friend class SingleSwitchProgramUI;
    template <class... Args>
    SingleSwitchProgramEnvironment(Logger& p_logger, Args&&... args)
        : logger(p_logger)
        , console(0, std::forward<Args>(args)...)
    {}
};


class SingleSwitchProgram : public RunnableProgram{
public:
    SingleSwitchProgram(
        FeedbackType feedback,
        PABotBaseLevel min_pabotbase,
        QString name,
        QString doc_link,
        QString description
    );

    virtual void program(SingleSwitchProgramEnvironment& env) const = 0;

private:
    SwitchSystemFactory m_switch;
};


class SingleSwitchProgramUI final : public RunnableProgramUI{
public:
    SingleSwitchProgramUI(SingleSwitchProgram& factory, MainWindow& window);
    ~SingleSwitchProgramUI();

    virtual void program() override;
};


template <typename Program>
class SingleSwitchProgramWrapper final : public Program{
public:
    SingleSwitchProgramWrapper() = default;
    SingleSwitchProgramWrapper(const QJsonValue& json)
        : Program()
    {
        this->from_json(json);
    }
    virtual QWidget* make_ui(MainWindow& window) override{
        return new SingleSwitchProgramUI(*this, window);
    }
};



}
}
#endif

