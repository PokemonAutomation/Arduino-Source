/*  Menu Option Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MenuOption_H
#define PokemonAutomation_PokemonSV_MenuOption_H

#include <array>
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_MenuOptionDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class MenuOption{
public:
    ~MenuOption();
    MenuOption(
        VideoStream& stream, ProControllerContext& context,
        Language language
    );

    // change all settings as per options
    void set_options(
        const std::vector<std::pair<MenuOptionItemEnum, std::vector<MenuOptionToggleEnum>>>& options) const;

private:

    // set the current selected option, to one of the option_toggle in the given list.
    // We need a list of option_toggle since the mapping for the options in each language is slightly different.
    //  - e.g. For "Send to Boxes", the options may be Manual/Automatic in English, but may be On/Off in other languages.
    // the program keeps cycling the options, until it matches one of the option_toggles in the list.
    void set_target_option(
        const std::vector<MenuOptionToggleEnum>& target_option_toggle_list
    ) const;

    //  Move to the target_option. 
    //  Returns empty string if not found.
    void move_to_option(const MenuOptionItemEnum target_option) const;

    std::string read_option(const ImageViewRGB32& cropped) const;

    int16_t read_number(Logger& logger, const ImageViewRGB32& cropped) const;

    // return the slug for the selected menu option item
    std::string read_option_item() const;

    // return the slug for the selected menu option toggle
    std::string read_option_toggle() const;

    // return the index row of the currently selected item.
    // this detects the gradient arrow.
    int8_t get_selected_index(const ImageViewRGB32& screen) const;
    

private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    Language m_language;
    VideoOverlaySet m_overlays;
    GradientArrowDetector m_arrow;
    std::array<ImageFloatBox, 10> m_boxes_item;
    std::array<ImageFloatBox, 10> m_boxes_toggle;
};




}
}
}
#endif
