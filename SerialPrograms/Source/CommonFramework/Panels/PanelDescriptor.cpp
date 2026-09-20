/*  Panel Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PanelDescriptor.h"
#include "PanelSession.h"

namespace PokemonAutomation{


PanelDescriptor::PanelDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    Color color,
    PanelDeprecation deprecation,
    bool restore_defaults_button,
    std::vector<std::string> required_resources
)
    : m_identifier(std::move(identifier))
    , m_category(std::move(category))
    , m_display_name(std::move(display_name))
    , m_doc_link(std::move(doc_link))
    , m_description(std::move(description))
    , m_color(color)
    , m_deprecation(deprecation)
    , m_restore_defaults_button(restore_defaults_button)
    , m_required_resources(std::move(required_resources))
{}
std::unique_ptr<PanelSession> PanelDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new PanelSession(*this));
}



}
