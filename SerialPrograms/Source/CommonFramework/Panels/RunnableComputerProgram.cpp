/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "RunnableComputerProgram.h"
#include "RunnableComputerProgramWidget.h"

namespace PokemonAutomation{


RunnableComputerProgramDescriptor::RunnableComputerProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description
)
    : RunnablePanelDescriptor(
        COLOR_DARKCYAN,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
{}

QWidget* RunnableComputerProgramInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return RunnableComputerProgramWidget::make(parent, *this, holder);
}






}
