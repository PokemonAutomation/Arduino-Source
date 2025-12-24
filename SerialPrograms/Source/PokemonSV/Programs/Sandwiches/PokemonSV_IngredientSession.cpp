/*  Ingredient Session
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
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_IngredientSession.h"
#include "Common/Cpp/PrettyPrint.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

IngredientSession::~IngredientSession() = default;

IngredientSession::IngredientSession(
    VideoStream& stream, ProControllerContext& context,
    Language language, SandwichIngredientType type
)
    : m_stream(stream)
    , m_context(context)
    , m_language(language)
    , m_overlays(stream.overlay())
    , m_type(type)
    , m_num_confirmed(0)  // Starts at 0, increments as ingredients are confirmed added
    , m_arrow(COLOR_CYAN, GradientArrowType::RIGHT, {0.02, 0.15, 0.05, 0.80})  // Detects cursor arrow
{
    SandwichIngredientReader reader(m_type, COLOR_CYAN);
    reader.make_overlays(m_overlays);
}



// Reads and analyzes one screenshot of the ingredient menu.
// Uses dual detection: OCR for all lines of ingredient names, and sprite matching for the cursor selected ingredient.
// This redundancy removes OCR ambiguity on the cursor selected ingredient.
//
// Process:
// 1. Detect cursor position using gradient arrow detector
// 2. Calculate which line (0-9) the cursor is on based on Y position
// 3. Run parallel OCR on all 10 lines to read ingredient names
// 4. Run sprite matching on the selected item's icon for confirmation
// 5. Cross-reference OCR and sprite results for the selected item to remove redundancy
//
// Returns: PageIngredients with detected cursor position and ingredient slugs.
//   It guarantees the cursor selected ingredient has no ambigurity (its line in PageIngredients::item[] has size of 1).
PageIngredients IngredientSession::read_screen(std::shared_ptr<const ImageRGB32> screen) const{
    PageIngredients ret;
    ImageFloatBox box;

    // Step 1: Detect the cyan gradient arrow that indicates cursor position
    if (!m_arrow.detect(box, *screen)){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "IngredientSession::read_current_page(): Unable to find cursor.",
            m_stream
        );
    }

    // Step 2: Calculate which line (slot) the cursor is on based on arrow Y position
    // Menu lines are evenly spaced, starting at Y=0.177778 with spacing of 0.0738683
    double slot = (box.y - 0.177778) / 0.0738683;
    ret.selected = (int8_t)(slot + 0.5);

    // Throws if cursor is not within expected range (0-9 for 10 lines per page)
    if (ret.selected < 0 || ret.selected >= 10){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "IngredientSession::read_current_page(): Invalid cursor slot.",
            m_stream,
            screen
        );
    }

    // Step 3 & 4: Run parallel OCR on all lines + sprite matching on selected item
    // Uses thread pool to speed up inference (11 tasks: 10 OCR + 1 sprite match)
    ImageMatch::ImageMatchResult image_result;
    SandwichIngredientReader reader(m_type);
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            if (index < SandwichIngredientReader::INGREDIENT_PAGE_LINES){
                // Task 0-9: OCR text detection on each of the 10 visible lines
                OCR::StringMatchResult result = reader.read_ingredient_page_with_ocr(*screen, m_stream.logger(), m_language, index);
                // Filter out low-confidence OCR matches
                result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
                result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
                // Store all plausible ingredient slugs for this line
                for (auto& item : result.results){
                    ret.item[index].insert(item.second.token);
                }
            }else{
                // Task 10: Sprite matching on the selected item's icon (for confirmation)
                image_result = reader.read_ingredient_page_with_icon_matcher(*screen, ret.selected);
                // Filter out low-confidence sprite matches
                image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
                image_result.log(m_stream.logger(), SandwichIngredientReader::MAX_ALPHA);
                image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
            }
        },
        0, SandwichIngredientReader::INGREDIENT_PAGE_LINES + 1
    );

#if 1
    // Step 5: Cross-reference OCR and sprite matching for the selected item
    // This disambiguation logic reduces false positives by requiring both methods to agree
    do{
        std::set<std::string>& ocr_result = ret.item[ret.selected];

        // Special case: If OCR completely failed but sprite matching found exactly one match,
        // trust the sprite result (OCR is less reliable than sprite matching)
        if (ocr_result.empty() || image_result.results.size() == 1){
            ocr_result.insert(image_result.results.begin()->second);
            break;
        }

        // Find ingredients that both OCR and sprite matching agree on
        // This intersection is our most confident result
        std::set<std::string> common;
        for (auto& item : image_result.results){
            auto iter = ocr_result.find(item.second);
            if (iter != ocr_result.end()){
                common.insert(item.second);
            }
        }

        // Error: OCR and sprite matching found no common ingredients
        // This indicates a serious detection failure
        if (common.empty()){
            std::set<std::string> sprite_result;
            for(const auto& p : image_result.results){
                sprite_result.insert(p.second);
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "IngredientSession::read_current_page(): Unable to read selected item. OCR and sprite do not agree on any match: ocr "
                + set_to_str(ocr_result) + ", sprite " + set_to_str(sprite_result),
                m_stream,
                screen
            );
        }

        // Error: Multiple ingredients match both OCR and sprite (ambiguous)
        if (common.size() > 1){
            std::set<std::string> sprite_result;
            for(const auto& p : image_result.results){
                sprite_result.insert(p.second);
            }
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "IngredientSession::read_current_page(): Unable to read selected item. Ambiguous result: "
                + set_to_str(ocr_result) + ", " + set_to_str(sprite_result) + "\n" + language_warning(m_language),
                m_stream,
                screen
            );
        }

        // Success: Exactly one ingredient matched by both methods
        // Replace OCR results with the confirmed ingredient
        ocr_result = std::move(common);
    }while (false);
#endif

    return ret;
}

PageIngredients IngredientSession::read_current_page() const{
    return read_screen(m_stream.video().snapshot());
}

bool IngredientSession::run_move_iteration(
    std::string& slug, const std::map<std::string, uint8_t>& ingredients,
    const PageIngredients& page
) const{
    size_t current_index = page.selected;

    // Find all desired ingredients on this page
    std::map<size_t, std::string> found_ingredients;  // line_index -> slug
    for (size_t c = 0; c < SandwichIngredientReader::INGREDIENT_PAGE_LINES; c++){
        for (const std::string& item : page.item[c]){
            if (ingredients.find(item) != ingredients.end()){
                found_ingredients[c] = item;
            }
        }
    }

    if (found_ingredients.empty()){
        return false;  // None of the desired ingredients are on this page
    }

    // Choose which ingredient to move to:
    // - If cursor is below all found ingredients, move to the closest (bottom-most) one
    // - Otherwise, move to the first (top-most) one
    size_t target_line_index = 0;
    if (std::all_of(found_ingredients.begin(), found_ingredients.end(),
                     [&](const auto& p){return p.first <= current_index;})){
        target_line_index = found_ingredients.rbegin()->first;  // Closest = bottom-most
    }else{
        target_line_index = found_ingredients.begin()->first;   // First = top-most
    }

    const std::string& item = found_ingredients[target_line_index];

    // Already on target ingredient
    if (current_index == target_line_index){
        m_stream.log("Desired ingredient " + item + " is selected!", COLOR_BLUE);
        slug = item;
        return true;
    }

    // Move cursor to target ingredient
    m_stream.log("Found desired ingredient " + item + " on current page. Moving towards it...", COLOR_BLUE);
    while (current_index < target_line_index){
        pbf_press_dpad(m_context, DPAD_DOWN, 10, 30);
        current_index++;
    }
    while (current_index > target_line_index){
        pbf_press_dpad(m_context, DPAD_UP, 10, 30);
        current_index--;
    }
    m_context.wait_for_all_requests();
    m_context.wait_for(std::chrono::seconds(1));
    return true;
}


std::string IngredientSession::move_to_ingredient(const std::map<std::string, uint8_t>& ingredients) const{
    if (ingredients.empty()){
        m_stream.log("No desired ingredients.", COLOR_RED);
        return "";
    }

    // TODO: Gin: we can delete the wait_for here?
    m_context.wait_for_all_requests();
    m_context.wait_for(std::chrono::milliseconds(180));

    size_t not_found_count = 0;  // Tracks full scans through menu
    while (true){
        // Read current page and try to move to ingredient
        PageIngredients page = read_current_page();
        std::string found_ingredient;
        if (run_move_iteration(found_ingredient, ingredients, page)){
            // We have found at least one ingredient on the current page:
            if (!found_ingredient.empty()){
                // The current cursor is already on a desired ingredient
                return found_ingredient;
            }
            // Desired ingredients found on page but not yet positioned, keep trying
            continue;
        }

        // Not found on current page. Check if at end of menu.
        // If you use DPAD_UP and DPAD_DOWN to move to the last line of the current page, the UI will always
        // show the next ingredient so that you are never at the last line of a page unless you really reach
        // the bottom of the menu.
        size_t current = page.selected;
        if (current == SandwichIngredientReader::INGREDIENT_PAGE_LINES - 1){  // On last line
            not_found_count++;
            if (not_found_count >= 2){
                // Scanned entire menu twice without finding ingredient
                m_stream.log("Ingredient not found anywhere.", COLOR_RED);
                return "";
            }else{
                // Wrap around to beginning for second pass
                m_stream.log("End of page reached without finding ingredient. Wrapping back to beginning.", COLOR_ORANGE);
                pbf_press_dpad(m_context, DPAD_DOWN, 160ms, 840ms);  // Wraps to top
                continue;
            }
        }

        m_stream.log("Ingredient not found on current page. Jumping down.", COLOR_ORANGE);
        // Press DPAD_RIGHT to jump to the next screen to have a full page of ingredients to check
        pbf_press_dpad(m_context, DPAD_RIGHT, 10, 30);
        m_context.wait_for_all_requests();
        m_context.wait_for(std::chrono::milliseconds(180));
    }
    return "";
}


void IngredientSession::add_ingredients(
    VideoStream& stream, ProControllerContext& context,
    std::map<std::string, uint8_t>&& ingredients  // Modified as items are added
){
    // Process ingredients until all are added
    while (!ingredients.empty()){
        // Navigate to any remaining ingredient
        std::string found = this->move_to_ingredient(ingredients);
        if (found.empty()){
            const SandwichIngredientNames& name = get_ingredient_name(ingredients.begin()->first);
            OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "Unable to find ingredient: \"" + name.display_name() + "\" - Did you run out?",
                stream
            );
        }

        const SandwichIngredientNames& name = get_ingredient_name(found);
        stream.log("Add " + name.display_name() + " as ingredient", COLOR_BLUE);

        // Add this ingredient the required number of times.
        // If you don't have enough number of this ingredient, it throws OperationFailedException.
        auto iter = ingredients.find(found);
        SandwichIngredientReader reader(m_type);
        while (iter->second > 0){  // While quantity remaining > 0
            bool ingredient_added = false;

            // Try up to 5 times to add and verify
            for (int attempt = 0; attempt < 5; attempt++){
                pbf_press_button(context, BUTTON_A, 160ms, 840ms);  // Add ingredient
                context.wait_for_all_requests();

                // Verify it appeared in confirmed list (bottom of screen)
                VideoSnapshot image = stream.video().snapshot();
                ImageMatch::ImageMatchResult image_result =
                    reader.read_confirmed_list_with_icon_matcher(image, m_num_confirmed);
                image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
                image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
                image_result.log(stream.logger(), SandwichIngredientReader::MAX_ALPHA);

                if (image_result.results.size() > 0){  // Confirmed ingredient was added
                    stream.overlay().add_log("Added " + name.display_name());
                    m_num_confirmed++;
                    ingredient_added = true;
                    iter->second--;  // Decrement quantity needed
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
        }
        ingredients.erase(iter);  // Remove fully-added ingredient from map
    }
}


void add_sandwich_ingredients(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>&& fillings,
    std::map<std::string, uint8_t>&& condiments
){
    // Phase 1: Add fillings
    {
        IngredientSession session(stream, context, language, SandwichIngredientType::FILLING);
        session.add_ingredients(stream, context, std::move(fillings));
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);  // Advance to condiments menu
    }

    // Phase 2: Add condiments
    {
        IngredientSession session(stream, context, language, SandwichIngredientType::CONDIMENT);

        // Special case: Herba Mystica ingredients are at the bottom of the list
        // Navigate to bottom first to avoid scrolling through entire menu
        if (std::any_of(condiments.begin(), condiments.end(),
                         [&](const auto& p){return p.first.find("herba") != std::string::npos;})){
            pbf_press_dpad(context, DPAD_UP, 160ms, 840ms);  // Wrap to bottom
        }

        session.add_ingredients(stream, context, std::move(condiments));
        pbf_press_button(context, BUTTON_PLUS, 160ms, 1840ms);  // Advance to pick menu
    }

    // Select the first pick and clear any remaining dialogs to go to stacking minigame.
    pbf_mash_button(context, BUTTON_A, 1000ms);
}






}
}
}
