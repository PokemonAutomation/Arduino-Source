/*  Panel Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PanelDescriptor.h"
#include "PanelInstance.h"

namespace PokemonAutomation{


PanelDescriptor::PanelDescriptor(
    Color color,
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    std::vector<ResourceType> required_resources
)
    : m_color(color)
    , m_identifier(std::move(identifier))
    , m_category(std::move(category))
    , m_display_name(std::move(display_name))
    , m_doc_link(std::move(doc_link))
    , m_description(std::move(description))
    , m_required_resources(std::move(required_resources))
{}
std::unique_ptr<PanelInstance> PanelDescriptor::make_panel() const{
    return std::unique_ptr<PanelInstance>(new PanelInstance(*this));
}



}
