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

class PanelInstance;

// Abstract base class that sets the interface for program descriptors.
// A program descriptor contains various information (descriptions) of a program panel UI.
// It can also use function `make_panel()` to create the corresponding panel.
class PanelDescriptor{
public:
    PanelDescriptor(
        Color color,
        std::string identifier,
        std::string category, std::string display_name,
        std::string doc_link,
        std::string description
    );
    virtual ~PanelDescriptor() = default;

    Color color() const{ return m_color; }
    const std::string& identifier() const{ return m_identifier; }
    const std::string& category() const{ return m_category; }
    const std::string& display_name() const{ return m_display_name; }
    const std::string& doc_link() const{ return m_doc_link; }
    const std::string& description() const{ return m_description; }

    virtual std::unique_ptr<PanelInstance> make_panel() const = 0;

private:
    const Color m_color;
    const std::string m_identifier;
    const std::string m_category;
    const std::string m_display_name;
    const std::string m_doc_link;
    const std::string m_description;
};


}
#endif
