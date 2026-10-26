/*  Panel Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PanelDescriptor.h"
#include "PanelSession.h"

namespace PokemonAutomation{



Color PanelDescriptor::pick_color(PanelQualityStanding standing){
    switch (standing){
    case PanelQualityStanding::OFFICIAL:
        return COLOR_BLUE;
    case PanelQualityStanding::OFFICIAL_BETA:
        return COLOR_DARKGREEN;
    case PanelQualityStanding::OFFICIAL_BELOW_STANDARDS:
        return COLOR_MAGENTA;
    case PanelQualityStanding::THIRD_PARTY:
        return COLOR_PURPLE;
    case PanelQualityStanding::THIRD_PARTY_UNVETTED:
        return COLOR_ORANGE;
    case PanelQualityStanding::UNMAINTAINED:
    case PanelQualityStanding::DEPRECATED:
        return COLOR_RED;
    case PanelQualityStanding::DEVELOPER:
        return COLOR_DARKCYAN;
    }
    return Color();
}

PanelDescriptor::PanelDescriptor(
    std::string identifier,
    std::string category, std::string display_name,
    std::string doc_link,
    std::string description,
    PanelQualityStanding standing,
    bool restore_defaults_button
)
    : m_identifier(std::move(identifier))
    , m_category(std::move(category))
    , m_display_name(std::move(display_name))
    , m_doc_link(std::move(doc_link))
    , m_description(std::move(description))
    , m_standing(standing)
    , m_color(pick_color(standing))
    , m_restore_defaults_button(restore_defaults_button)
{}
std::unique_ptr<PanelSession> PanelDescriptor::make_panel() const{
    return std::unique_ptr<PanelSession>(new PanelSession(*this));
}



}
