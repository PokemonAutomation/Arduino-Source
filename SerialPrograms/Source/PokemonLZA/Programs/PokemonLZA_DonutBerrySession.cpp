/*  Donut Berry Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerrySession.h"
#include "Common/Cpp/PrettyPrint.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

BerrySession::~BerrySession() = default;

BerrySession::BerrySession(
    VideoStream& stream, ProControllerContext& context,
    Language language
)
    : m_stream(stream)
    , m_context(context)
    , m_language(language)
    , m_overlays(stream.overlay())
    //, m_num_confirmed(0)
{
    DonutBerriesReader reader(COLOR_CYAN);
    reader.make_overlays(m_overlays);
}



PageIngredients BerrySession::read_screen(std::shared_ptr<const ImageRGB32> screen) const{
    PageIngredients ret;
    
    int slot = 0;
    for (int i = 0; i < 8; i++) {
        DonutBerriesSelectionWatcher arrow(i);
        //cout << "menu index: " << i << endl;
        if (arrow.detect(*screen)) {
            //cout << "detected in slot: " << i << endl;
            slot = i;
            break;
        }
    }
    ret.selected = (int8_t)slot;
    //cout << "selected slot = " << (int)ret.selected << endl;
    if (ret.selected < 0 || ret.selected >= 8){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "BerrySession::read_current_page(): Invalid cursor slot.",
            m_stream,
            screen
        );
    }
    
    //  Read the names of every line and the sprite of the selected line.
    ImageMatch::ImageMatchResult image_result;
    DonutBerriesReader reader;
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            if (index < DonutBerriesReader::BERRY_PAGE_LINES){
                // Read text at line `index`
                OCR::StringMatchResult result = reader.read_berry_page_with_ocr(*screen, m_stream.logger(), m_language, index);
                result.clear_beyond_log10p(DonutBerriesOCR::MAX_LOG10P);
                result.clear_beyond_spread(DonutBerriesOCR::MAX_LOG10P_SPREAD);
                for (auto& item : result.results){
                    ret.item[index].insert(item.second.token);
                }
            }else{
                // Read current selected icon
                image_result = reader.read_berry_page_with_icon_matcher(*screen, ret.selected);
                image_result.clear_beyond_spread(DonutBerriesReader::ALPHA_SPREAD);
                image_result.log(m_stream.logger(), DonutBerriesReader::MAX_ALPHA);
                image_result.clear_beyond_alpha(DonutBerriesReader::MAX_ALPHA);
            }
        },
        0, DonutBerriesReader::BERRY_PAGE_LINES + 1
    );

#if 0 //Todo: Image match isn't the best since quantity covers it
    do{
        std::set<std::string>& ocr_result = ret.item[ret.selected];

        //  Special case where OCR fails and image result returns 1 item.
        if (ocr_result.empty() || image_result.results.size() == 1){
            ocr_result.insert(image_result.results.begin()->second);
            break;
        }

        //  Find the items in common between the two detection methods.
        std::set<std::string> common;
        for (auto& item : image_result.results){
            auto iter = ocr_result.find(item.second);
            if (iter != ocr_result.end()){
                common.insert(item.second);
            }
        }

        if (common.empty()){
            std::set<std::string> sprite_result;
            for(const auto& p : image_result.results){
                sprite_result.insert(p.second);
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "BerrySession::read_current_page(): Unable to read selected item. OCR and sprite do not agree on any match: ocr "
                + set_to_str(ocr_result) + ", sprite " + set_to_str(sprite_result),
                m_stream,
                screen
            );
        }
        if (common.size() > 1){
            std::set<std::string> sprite_result;
            for(const auto& p : image_result.results){
                sprite_result.insert(p.second);
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "BerrySession::read_current_page(): Unable to read selected item. Ambiguous result: "
                + set_to_str(ocr_result) + ", " + set_to_str(sprite_result) + "\n" + language_warning(m_language),
                m_stream,
                screen
            );
        }

        ocr_result = std::move(common);
    }while (false);
#endif

    return ret;

}
PageIngredients BerrySession::read_current_page() const{
    return read_screen(m_stream.video().snapshot());
}

//  Returns true if a desired ingredient is found somewhere on the page.
//  "slug" is set the desired ingredient if the cursor is already on it.
bool BerrySession::run_move_iteration(
    std::string& slug, const std::set<std::string>& ingredients,
    const PageIngredients& page
) const{
    size_t current_index = page.selected;
    std::map<size_t, std::string> found_ingredients;
    for (size_t c = 0; c < DonutBerriesReader::BERRY_PAGE_LINES; c++){
        for (const std::string& item : page.item[c]){
            //auto iter = ingredients.find(item);
            if (ingredients.find(item) != ingredients.end()) {
                found_ingredients[c] = item;
            }
        }
    }

    if (found_ingredients.size() == 0){
        return false;
    }

    size_t target_line_index = 0;
    if (std::all_of(found_ingredients.begin(), found_ingredients.end(), [&](const auto& p){return p.first <= current_index;})){
        // If the current cursor is below all the found ingredients,
        // we should move to the closest ingredient, which is also at the lowest line among found ingredients.
        target_line_index = found_ingredients.rbegin()->first;
    }else{
        target_line_index = found_ingredients.begin()->first;
    }
    
    //cout << "current index: " << current_index << endl;
    //cout << "target line index: " << target_line_index << endl;

    const std::string& item = found_ingredients[target_line_index];

    //  Cursor is already on matching ingredient.
    if (current_index == target_line_index){
        m_stream.log("Desired ingredient " + item + " is selected!", COLOR_BLUE);
        slug = item;
        return true;
    }

    m_stream.log("Found desired ingredient " + item + " on current page. Moving towards it...", COLOR_BLUE);

    //  Move to it.

    while (current_index < target_line_index){
        pbf_press_dpad(m_context, DPAD_DOWN, 80ms, 240ms);
        current_index++;
    }
    while (current_index > target_line_index){
        pbf_press_dpad(m_context, DPAD_UP, 80ms, 240ms);
        current_index--;
    }
    m_context.wait_for_all_requests();
    m_context.wait_for(std::chrono::seconds(1));
    // slug = item;
    return true;
}


std::string BerrySession::move_to_ingredient(const std::set<std::string>& ingredients) const{
    if (ingredients.empty()){
        m_stream.log("No desired ingredients.", COLOR_RED);
        return "";
    }

    size_t not_found_count = 0;
    while (true){
        m_context.wait_for_all_requests();
        m_context.wait_for(std::chrono::milliseconds(500));
        PageIngredients page = read_current_page();
        std::string found_ingredient;
        if (run_move_iteration(found_ingredient, ingredients, page)){
            if (found_ingredient.empty()){
                continue;
            }else{
                return found_ingredient;
            }
            continue;
        }

        size_t current = page.selected;
        if (current == DonutBerriesReader::BERRY_PAGE_LINES - 1){
            not_found_count++;
            if (not_found_count >= 2){
                m_stream.log("Ingredient not found anywhere.", COLOR_RED);
                return "";
            }else{
                m_stream.log("End of page reached without finding ingredient. Wrapping back to beginning.", COLOR_ORANGE);
                pbf_press_dpad(m_context, DPAD_DOWN, 160ms, 840ms);
                continue;
            }
        }

        m_stream.log("Ingredient not found on current page. Scrolling up.", COLOR_ORANGE);

        //  Not found on page. Scroll to next screen
        pbf_press_dpad(m_context, DPAD_RIGHT, 80ms, 240ms);
        m_context.wait_for_all_requests();
        m_context.wait_for(std::chrono::milliseconds(180));
    }
    return "";
}


void BerrySession::add_ingredients(
    VideoStream& stream, ProControllerContext& context,
    std::map<std::string, uint8_t>&& ingredients
){
    //  "ingredients" will be what we still need.
    //  Each time we add an ingredient, it will be removed from the map.
    //  Loop until there's nothing left.
    while (!ingredients.empty()){
        std::set<std::string> remaining;
        for (const auto& item : ingredients){
            remaining.insert(item.first);
        }

        std::string found = this->move_to_ingredient(remaining);
        if (found.empty()){
            const DonutBerries& name = get_berry_name(*remaining.begin());
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Unable to find ingredient: \"" + name.display_name() + "\" - Did you run out?",
                stream
            );
        }

        const DonutBerries& name = get_berry_name(found);
        stream.log("Add " + name.display_name() + " as ingredient", COLOR_BLUE);

        //  If you don't have enough ingredient, it errors out instead of proceeding 
        //  with less than the desired quantity.
        auto iter = ingredients.find(found);
        DonutBerriesReader reader;
        while (iter->second > 0){
            pbf_press_button(context, BUTTON_A, 160ms, 840ms);
            context.wait_for_all_requests();
            iter->second--;

            /* Todo: Image match isn't the best since B button covers it
            bool ingredient_added = false;
            for (int attempt = 0; attempt < 5; attempt++){
                pbf_press_button(context, BUTTON_A, 160ms, 840ms);
                context.wait_for_all_requests();
                VideoSnapshot image = stream.video().snapshot();
                ImageMatch::ImageMatchResult image_result = 
                    reader.read_confirmed_list_with_icon_matcher(image, m_num_confirmed);
                image_result.clear_beyond_spread(DonutBerriesReader::ALPHA_SPREAD);
                image_result.clear_beyond_alpha(DonutBerriesReader::MAX_ALPHA);
                image_result.log(stream.logger(), DonutBerriesReader::MAX_ALPHA);
                if (image_result.results.size() > 0){ // confirmed that the ingredient was added
                    stream.overlay().add_log("Added " + name.display_name());
                    m_num_confirmed++;
                    ingredient_added = true;
                    iter->second--;
                    break;
                }
            }

            if (!ingredient_added){
                OperationFailedException::fire(
                    ErrorReport::NO_ERROR_REPORT,
                    "Unable to add ingredient: \"" + name.display_name() + "\" - Did you run out?",
                    stream
                );
            }
            */
        }
        ingredients.erase(iter);
    }
}



void add_donut_ingredients(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>&& fillings
){
    BerrySession session(stream, context, language);
    pbf_press_dpad(context, DPAD_UP, 160ms, 840ms); //Starting from the bottom, assuming users want hyperspace berries
    session.add_ingredients(stream, context, std::move(fillings));
    pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);
}






}
}
}
