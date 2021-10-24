/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnableComputerProgram_H
#define PokemonAutomation_RunnableComputerProgram_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "RunnablePanel.h"

namespace PokemonAutomation{

class RunnableComputerProgramDescriptor : public RunnablePanelDescriptor{
public:
    RunnableComputerProgramDescriptor(
        std::string identifier,
        QString display_name,
        QString doc_link,
        QString description
    );
};



class RunnableComputerProgramInstance : public RunnablePanelInstance{
public:
    using RunnablePanelInstance::RunnablePanelInstance;

    const RunnableComputerProgramDescriptor& descriptor() const{
        return static_cast<const RunnableComputerProgramDescriptor&>(m_descriptor);
    }

    virtual QWidget* make_widget(QWidget& parent, PanelListener& listener) override;
    virtual void program(ProgramEnvironment& env) = 0;
};



class RunnableComputerProgramWidget : public RunnablePanelWidget{
public:
    static RunnableComputerProgramWidget* make(
        QWidget& parent,
        RunnableComputerProgramInstance& instance,
        PanelListener& listener
    );

private:
    using RunnablePanelWidget::RunnablePanelWidget;
    virtual ~RunnableComputerProgramWidget();

    virtual void run_program() override final;
};



}
#endif
