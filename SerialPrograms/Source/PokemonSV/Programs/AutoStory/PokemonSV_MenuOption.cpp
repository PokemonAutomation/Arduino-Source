/*  Menu Option Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_MenuOption.h"
#include "PokemonSV/Inference/PokemonSV_MenuOptionReader.h"

//#include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

MenuOption::~MenuOption() = default;

MenuOption::MenuOption(
    VideoStream& stream,
    ProControllerContext& context,
    Language language
)
    : m_stream(stream)
    , m_context(context)
    , m_language(language)
    , m_overlays(stream.overlay())
    , m_arrow(COLOR_CYAN, GradientArrowType::RIGHT, {0.02, 0.10, 0.05, 0.80})
{
    // reader.make_overlays(m_overlays);
    for (size_t c = 0; c < 10; c++){
        m_boxes_item[c] = ImageFloatBox(0.055, 0.135 + c * 0.0739, 0.260, 0.060);
        m_boxes_toggle[c] = ImageFloatBox(0.345, 0.135 + c * 0.0739, 0.175, 0.060);
        m_overlays.add(COLOR_BLUE, m_boxes_item[c]);
        m_overlays.add(COLOR_BLUE, m_boxes_toggle[c]);
    }
}

void MenuOption::set_options(
    const std::vector<std::pair<MenuOptionItemEnum, std::vector<MenuOptionToggleEnum>>>& options
) const{
    for (std::pair<MenuOptionItemEnum, std::vector<MenuOptionToggleEnum>> option : options){
        move_to_option(option.first);
        set_target_option(option.second);
    }
}

void MenuOption::set_target_option(const std::vector<MenuOptionToggleEnum>& target_option_toggle_list) const{

    for (size_t attempt = 0; attempt < 10; attempt++){
        std::string current_option_slug = read_option_toggle();
        MenuOptionToggleEnum current_option_toggle_enum = menu_option_toggle_lookup_by_slug(current_option_slug).enum_value;
        
        if (std::find(target_option_toggle_list.begin(), target_option_toggle_list.end(), current_option_toggle_enum) != target_option_toggle_list.end()){
            return;
        }

        pbf_press_dpad(m_context, DPAD_RIGHT, 10, 50);
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "MenuOption::set_target_option(): Unable to set option to the correct toggle.",
        m_stream
    );

}

int8_t MenuOption::get_selected_index(const ImageViewRGB32& screen) const {
    m_context.wait_for_all_requests();
    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "MenuOption::get_selected_index(): Unable to find cursor.\n"
            "We expect to be in the Options screen. Ensure you selected the correct Autostory start segment.",
            m_stream
        );
    }

//    cout << box.y << endl;
    double slot = (box.y - 0.135185) / 0.0739;
    int8_t selected_index = (int8_t)(slot + 0.5);

    if (selected_index < 0 || selected_index >= 10){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "MenuOption::get_selected_index(): Invalid cursor slot.",
            m_stream
        );
    }
    // cout << "selected index:" + std::to_string(selected_index) << endl;

    return selected_index;
}

std::string MenuOption::read_option(const ImageViewRGB32& cropped) const{
    // cropped.save("test.png");
    const auto ocr_result = MenuOptionReader::instance().read_substring(
        m_stream.logger(),
        m_language,
        cropped, OCR::BLACK_TEXT_FILTERS()
    );

    std::multimap<double, OCR::StringMatchData> results;
    if (!ocr_result.results.empty()){
        for (const auto& result : ocr_result.results){
            results.emplace(result.first, result.second);
        }
    }

    if (results.empty()){
        if (m_language == Language::German){
            // German Menu Option uses numbers for text speed settings, which string OCR has trouble with detecting.
            int16_t number = read_number(m_stream.logger(), cropped);
            switch (number){
                case 1:
                    return "slow";
                case 2:
                    return "normal";
                case 3:
                    return "fast";
            }
        }
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "MenuOption::read_option(): Unable to read item. No results returned.",
            m_stream
        );        
    }

    if (results.size() > 1){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "MenuOption::read_option(): Unable to read item. Ambiguous or multiple results.\n" + language_warning(m_language),
            m_stream
        );
    }

    return results.begin()->second.token;
}

int16_t MenuOption::read_number(
    Logger& logger, 
    const ImageViewRGB32& cropped
) const{

    int16_t number = (int16_t)OCR::read_number_waterfill(logger, cropped, 0xff000000, 0xff808080);

    if (number < 1 || number > 3){
        number = -1; 
    }
    return (int16_t)number;
}

std::string MenuOption::read_option_item() const{
    m_context.wait_for_all_requests();
    VideoSnapshot screen = m_stream.video().snapshot();
    
    ImageFloatBox selected_box = m_boxes_item[get_selected_index(screen)];
    ImageViewRGB32 cropped = extract_box_reference(screen, selected_box);
    return read_option(cropped);

}

std::string MenuOption::read_option_toggle() const{
    m_context.wait_for_all_requests();
    VideoSnapshot screen = m_stream.video().snapshot();

    ImageFloatBox selected_box = m_boxes_toggle[get_selected_index(screen)];
    ImageViewRGB32 cropped = extract_box_reference(screen, selected_box);

    return read_option(cropped);
}


void MenuOption::move_to_option(const MenuOptionItemEnum target_option_enum) const{
    std::string current_option_slug = read_option_item();
    MenuOptionItem current_option = menu_option_item_lookup_by_slug(current_option_slug);

    const MenuOptionItem& target_option = menu_option_item_lookup_by_enum(target_option_enum);
    std::string target_option_slug = target_option.slug;
    int diff = target_option.index - current_option.index;
    // cout << "target_option diff: " << std::to_string(target_option.index) << endl;
    // cout << "current_option diff: " << std::to_string(current_option.index) << endl;
    // cout << "diff: " << std::to_string(diff) << endl;
    while (diff != 0){
        // move cursor in direction of target_option
        if (diff > 0){
            for (size_t i = 0; i < (size_t)diff; i++){
                pbf_press_dpad(m_context, DPAD_DOWN, 10, 50);
            }
        }
        if (diff < 0){
            for (size_t i = 0; i < (size_t)-diff; i++){
                pbf_press_dpad(m_context, DPAD_UP, 10, 50);
            }
        }
        current_option_slug = read_option_item();
        current_option = menu_option_item_lookup_by_slug(current_option_slug);
        diff = target_option.index - current_option.index;
        // cout << "diff: " << std::to_string(diff) << endl;
    }
}






}
}
}
