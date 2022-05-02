/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_RunnableComputerProgramWidget_H
#define PokemonAutomation_RunnableComputerProgramWidget_H

#include "RunnablePanelWidget.h"
#include "RunnableComputerProgram.h"

namespace PokemonAutomation{



class RunnableComputerProgramWidget : public RunnablePanelWidget{
public:
    static RunnableComputerProgramWidget* make(
        QWidget& parent,
        RunnableComputerProgramInstance& instance,
        PanelHolder& holder
    );

private:
    using RunnablePanelWidget::RunnablePanelWidget;
    virtual ~RunnableComputerProgramWidget();

    virtual void run_program() override final;
};




}
#endif
