/*  Runnable Computer Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ComputerProgram.h"
#include "ComputerPrograms/Framework/ComputerProgramSession.h"

namespace PokemonAutomation{


ComputerProgramDescriptor::ComputerProgramDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelDeprecation deprecation
)
    : ProgramDescriptor(
        COLOR_DARKCYAN,
        std::move(identifier),
        std::move(category), std::move(display_name),
        std::move(doc_link),
        std::move(description),
        deprecation,
        true,
        {}
    )
{}
std::unique_ptr<PanelSession> ComputerProgramDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new ComputerProgramSession(*this));
}









}
