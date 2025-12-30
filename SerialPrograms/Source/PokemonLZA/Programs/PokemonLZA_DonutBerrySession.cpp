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

//  Returns true if the target berry is found somewhere on the page.
//  "slug" is set the target berry if the cursor is already on it.
bool BerrySession::run_move_iteration(
    std::string& slug, const std::string& berry,
    const PageIngredients& page
) const{
    size_t current_index = page.selected;
    size_t found_location = SIZE_MAX;
    for (size_t c = 0; c < DonutBerriesReader::BERRY_PAGE_LINES; c++){
        for (const std::string& item : page.item[c]){
            if (berry == item) {
                found_location = c;
                break;
            }
        }
        if (found_location != SIZE_MAX){
            break;
        }
    }

    if (found_location == SIZE_MAX){
        return false;
    }
    
    //  Cursor is already on matching ingredient.
    if (current_index == found_location){
        m_stream.log("Desired " + berry + " is selected!", COLOR_BLUE);
        slug = berry;
        return true;
    }

    m_stream.log("Found desired " + berry + " on current page. Moving towards it...", COLOR_BLUE);

    //  Move to it.
    while (current_index < found_location){
        pbf_press_dpad(m_context, DPAD_DOWN, 80ms, 200ms);
        current_index++;
    }
    while (current_index > found_location){
        pbf_press_dpad(m_context, DPAD_UP, 80ms, 200ms);
        current_index--;
    }
    m_context.wait_for_all_requests();
    m_context.wait_for(Milliseconds(500));
    return true;
}


std::string BerrySession::move_to_ingredient(const std::string& berry, bool move_topdown) const{
    size_t not_found_count = 0;
    while (true){
        m_context.wait_for_all_requests();
        PageIngredients page = read_current_page();
        std::string found_ingredient;
        if (run_move_iteration(found_ingredient, berry, page)){
            if (found_ingredient.empty()){
                m_context.wait_for(std::chrono::milliseconds(100));
                continue;
            }
            return found_ingredient;
        }

        size_t current = page.selected;
        if ((move_topdown && current == DonutBerriesReader::BERRY_PAGE_LINES - 1) || 
            (!move_topdown && current == 0)){
            not_found_count++;
            if (not_found_count >= 2){
                m_stream.log("Berry not found anywhere.", COLOR_RED);
                return "";
            }
            m_stream.log("End of page reached without finding " + berry + ". Wrapping back.", COLOR_ORANGE);
        }

        m_stream.log("Ingredient not found on current page. Jump to next page.", COLOR_ORANGE);

        //  Not found on page. Jump to next screen
        pbf_press_dpad(m_context, move_topdown ? DPAD_RIGHT : DPAD_LEFT, 80ms, 240ms);
        m_context.wait_for_all_requests();
        m_context.wait_for(std::chrono::milliseconds(100));
    }
    return "";
}


void BerrySession::add_berries(
    VideoStream& stream, ProControllerContext& context,
    const std::map<std::string, uint8_t>& berries
){
    std::map<std::string, size_t> BERRY_ORDER;
    for (size_t i = 0; i < DONUT_BERRIES_SLUGS().size(); i++){
        BERRY_ORDER.emplace(DONUT_BERRIES_SLUGS()[i], i);
    }
    const size_t NUM_NORMAL_BERRIES = DONUT_BERRIES_SLUGS().size() / 2;
    const size_t NUM_HYPER_BERRIES = NUM_NORMAL_BERRIES;

    bool all_hyper_berries = true;
    bool all_normal_berries = true;
    std::string log_msg = "Berries to add: ";
    for(const auto& p : berries){
        log_msg += p.first + ": " + std::to_string(p.second) + ", ";
        if (BERRY_ORDER.find(p.first) == BERRY_ORDER.end()){
            throw InternalProgramError(&stream.logger(), 
                PA_CURRENT_FUNCTION, "unknown berry slug " + p.first);
        }
        if (p.first.starts_with("hyper-")){
            all_normal_berries = false;
        } else{
            all_hyper_berries = false;
        }
    }
    stream.log(log_msg.substr(0, log_msg.size() - 2)); // -2 to remove the trailing ", "

    // Compute whether we should go from top to bottom or bottom to top to search for berries:
    
    std::vector<std::pair<std::string, uint8_t>> ordered_berries;

    for (size_t i = 0; i < DONUT_BERRIES_SLUGS().size(); i++){
        const auto& berry_slug = DONUT_BERRIES_SLUGS()[i];
        auto it = berries.find(berry_slug);
        if (it != berries.end()){
            ordered_berries.emplace_back(berry_slug, it->second);
        }
    }
    const size_t total_movement_topdown = BERRY_ORDER[ordered_berries.back().first] - (all_hyper_berries ? NUM_NORMAL_BERRIES : 0);
    const size_t total_movment_bottomup = BERRY_ORDER.size() - BERRY_ORDER[ordered_berries.front().first] - (all_normal_berries ? NUM_HYPER_BERRIES : 0);
    bool move_topdown = total_movement_topdown <= total_movment_bottomup;
    if (all_hyper_berries && move_topdown){
        // Press minus button twice to change to Hyperspace berry-only menu
        pbf_press_button(context, BUTTON_MINUS, 100ms, 500ms);
        pbf_press_button(context, BUTTON_MINUS, 100ms, 700ms);
        context.wait_for_all_requests();
    } else if (all_normal_berries && !move_topdown){
        // Press minus button once to change to normal berry-only menu
        pbf_press_button(context, BUTTON_MINUS, 100ms, 700ms);
        context.wait_for_all_requests();
    }
    
    if (move_topdown){
        // reverse the order so that we can pop back
        std::reverse(ordered_berries.begin(), ordered_berries.end());
    } else{
        // Move to bottom
        pbf_press_dpad(context, DPAD_UP, 100ms, 500ms);
        context.wait_for_all_requests();
    }

    while (!ordered_berries.empty()){
        const auto& back = ordered_berries.back();
        std::string found = this->move_to_ingredient(back.first, move_topdown);
        if (found.empty()){
            const DonutBerries& name = get_berry_name(berries.begin()->first);
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Unable to find Berry: \"" + name.display_name() + "\" - Did you run out?",
                stream
            );
        }

        const DonutBerries& name = get_berry_name(found);
        stream.log("Adding " + name.display_name() + " x " + std::to_string(back.second), COLOR_BLUE);

        DonutBerriesReader reader;
        for(size_t i = 0; i < back.second; i++){
            pbf_press_button(context, BUTTON_A, 160ms, 400ms);
            context.wait_for_all_requests();
            stream.overlay().add_log("Add " + name.display_name());

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
        ordered_berries.pop_back();
    } // end while (!ordered_berries.empty())
}



void add_donut_berries(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::map<std::string, uint8_t>& berries
){
    BerrySession session(stream, context, language);
    session.add_berries(stream, context, berries);
    pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);
    context.wait_for_all_requests();
}






}
}
}
