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
    QString category, QString display_name,
    QString doc_link,
    QString description
)
    : RunnablePanelDescriptor(
        Qt::darkCyan,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description)
    )
{}


QWidget* RunnableComputerProgramInstance::make_widget(QWidget& parent, PanelListener& listener){
    return RunnableComputerProgramWidget::make(parent, *this, listener);
}






}
