/*  Panel Descriptor
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PanelDescriptor_H
#define PokemonAutomation_PanelDescriptor_H

#include <memory>
#include <string>
#include "Common/Cpp/Color.h"

namespace PokemonAutomation{

class PanelSession;


enum class PanelQualityStanding{
    OFFICIAL,
    OFFICIAL_BETA,
    OFFICIAL_BELOW_STANDARDS,
    THIRD_PARTY,
    THIRD_PARTY_UNVETTED,
    UNMAINTAINED,
    DEPRECATED,
    DEVELOPER,
//    UNCATEGORIZED,
};


// Abstract base class that sets the interface for program descriptors.
// A program descriptor contains various information (descriptions) of a program panel UI.
// It can also use function `make_panel()` to create the corresponding panel.
class PanelDescriptor{
public:
    PanelDescriptor(
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description,
        PanelQualityStanding standing,
        bool restore_defaults_button
    );
    virtual ~PanelDescriptor() = default;

    const std::string& identifier() const{ return m_identifier; }
    const std::string& category() const{ return m_category; }
    const std::string& display_name() const{ return m_display_name; }
    const std::string& doc_link() const{ return m_doc_link; }
    const std::string& description() const{ return m_description; }
    PanelQualityStanding standing() const{ return m_standing; }

    Color color() const{ return m_color; }
    bool restore_defaults_button() const{ return m_restore_defaults_button; }

    virtual std::unique_ptr<PanelSession> make_panel() const = 0;


public:
    static Color pick_color(PanelQualityStanding standing);


private:
    const std::string m_identifier;
    const std::string m_category;
    const std::string m_display_name;
    const std::string m_doc_link;
    const std::string m_description;
    const PanelQualityStanding m_standing;
    const Color m_color;
    const bool m_restore_defaults_button;
};


}
#endif
