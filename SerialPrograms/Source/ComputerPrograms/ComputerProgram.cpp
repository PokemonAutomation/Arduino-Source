/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ComputerPrograms/Framework/ComputerProgramSession.h"
#include "ComputerProgram.h"

namespace PokemonAutomation{


ComputerProgramDescriptor::ComputerProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelQualityStanding standing
)
    : ProgramDescriptor(
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        standing,
        true,
        {}
    )
{}
std::unique_ptr<PanelSession> ComputerProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new ComputerProgramSession(*this));
}









}
