/*  Ingredient Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_IngredientSession.h"
#include "Common/Cpp/PrettyPrint.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

IngredientSession::~IngredientSession() = default;

IngredientSession::IngredientSession(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language, SandwichIngredientType type
)
    : m_dispatcher(dispatcher)
    , m_console(console)
    , m_context(context)
    , m_language(language)
    , m_overlays(console.overlay())
    , m_ingredients(10)
    , m_arrow(COLOR_CYAN, GradientArrowType::RIGHT, {0.02, 0.15, 0.05, 0.80})
{
    for (size_t c = 0; c < INGREDIENT_PAGE_LINES; c++){
        m_ingredients.emplace_back(type, c, COLOR_CYAN);
        m_ingredients.back().make_overlays(m_overlays);
    }
}


PageIngredients IngredientSession::read_current_page() const{
    VideoSnapshot snapshot = m_console.video().snapshot();

    PageIngredients ret;
    ImageFloatBox box;
    if (!m_arrow.detect(box, snapshot)){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Unable to find cursor.",
            snapshot
        );
    }

//    cout << box.y << endl;
    double slot = (box.y - 0.177778) / 0.0738683;
    ret.selected = (int8_t)(slot + 0.5);
//    cout << "slot = " << (int)ret.selected << endl;
    if (ret.selected < 0 || ret.selected >= 10){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Invalid cursor slot.",
            snapshot
        );
    }

    //  Read the names of every line and the sprite of the selected line.
    ImageMatch::ImageMatchResult image_result;
    m_dispatcher.run_in_parallel(0, INGREDIENT_PAGE_LINES + 1, [&](size_t index){
        if (index < INGREDIENT_PAGE_LINES){
            // Read text at line `index`
            OCR::StringMatchResult result = m_ingredients[index].read_with_ocr(snapshot, m_console, m_language);
            result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
            result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
            for (auto& item : result.results){
                ret.item[index].insert(item.second.token);
            }
        }else{
            // Read current selected icon
            image_result = m_ingredients[ret.selected].read_with_icon_matcher(snapshot);
            image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
            image_result.log(m_console, SandwichIngredientReader::MAX_ALPHA);
            image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
        }
    });

#if 1
    std::set<std::string>& ocr_result = ret.item[ret.selected];
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
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Unable to read selected item. OCR and sprite do not agree on any match: ocr "
            + set_to_str(ocr_result) + ", sprite " + set_to_str(sprite_result),
            snapshot
        );
    }
    if (common.size() > 1){
        std::set<std::string> sprite_result;
        for(const auto& p : image_result.results){
            sprite_result.insert(p.second);
        }
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Unable to read selected item. Ambiguous result: "
            + set_to_str(ocr_result) + ", " + set_to_str(sprite_result),
            snapshot
        );
    }

    ocr_result = std::move(common);
#endif

    return ret;
}

//  Returns true if a desired ingredient is found somewhere on the page.
//  "slug" is set the desired ingredient if the cursor is already on it.
bool IngredientSession::run_move_iteration(
    std::string& slug, const std::set<std::string>& ingredients,
    const PageIngredients& page
) const{
    size_t current_index = page.selected;
    std::map<size_t, std::string> found_ingredients;
    for (size_t c = 0; c < INGREDIENT_PAGE_LINES; c++){
        for (const std::string& item : page.item[c]){
            auto iter = ingredients.find(item);
            if (iter != ingredients.end()){
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
    } else {
        target_line_index = found_ingredients.begin()->first;
    }

    const std::string& item = found_ingredients[target_line_index];

    //  Cursor is already on matching ingredient.
    if (current_index == target_line_index){
        m_console.log("Desired ingredient " + item + " is selected!", COLOR_BLUE);
        slug = item;
        return true;
    }

    m_console.log("Found desired ingredient " + item + " on current page. Moving towards it...", COLOR_BLUE);

    //  Move to it.
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


std::string IngredientSession::move_to_ingredient(const std::set<std::string>& ingredients) const{
    if (ingredients.empty()){
        m_console.log("No desired ingredients.", COLOR_RED);
        return "";
    }

    size_t not_found_count = 0;
    while (true){
        m_context.wait_for_all_requests();
        PageIngredients page = read_current_page();

        std::string found_ingredient;
        if (run_move_iteration(found_ingredient, ingredients, page)){
            if (found_ingredient.empty()){
                continue;
            }else{
                return found_ingredient;
            }
        }

        size_t current = page.selected;
        if (current == INGREDIENT_PAGE_LINES - 1){
            not_found_count++;
            if (not_found_count >= 2){
                m_console.log("Ingredient not found anywhere.", COLOR_RED);
                return "";
            }else{
                m_console.log("End of page reached without finding ingredient. Wrapping back to beginning.", COLOR_ORANGE);
                pbf_press_dpad(m_context, DPAD_DOWN, 20, 105);
            }
        }

        m_console.log("Ingredient not found on current page. Scrolling down.", COLOR_ORANGE);

        //  Not found on page. Scroll to last item.
        while (current < INGREDIENT_PAGE_LINES - 1){
            pbf_press_dpad(m_context, DPAD_DOWN, 10, 30);
            current++;
        }

        m_context.wait_for_all_requests();
        m_context.wait_for(std::chrono::milliseconds(180));
    }
    return "";
}


void IngredientSession::add_ingredients(
    ConsoleHandle& console, BotBaseContext& context,
    std::map<std::string, uint8_t>&& ingredients
) const{
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
            const SandwichIngredientNames& name = get_ingredient_name(*remaining.begin());
            throw OperationFailedException(
                ErrorReport::NO_ERROR_REPORT, console,
                "Unable to find ingredient: \"" + name.display_name() + "\" - Did you run out?"
            );
        }

        const SandwichIngredientNames& name = get_ingredient_name(found);
        console.log("Found: " + name.display_name(), COLOR_BLUE);

        //  Add the item. But don't loop the quantity. Instead, we add one and
        //  loop again in case we run out.
        pbf_press_button(context, BUTTON_A, 20, 105);
        context.wait_for_all_requests();
        // TODO add visual confirmation of ingredients added to avoid button drops

        auto iter = ingredients.find(found);
        if (--iter->second == 0){
            ingredients.erase(iter);
        }
    }
}



void add_sandwich_ingredients(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language,
    std::map<std::string, uint8_t>&& fillings,
    std::map<std::string, uint8_t>&& condiments
){
    {
        IngredientSession session(dispatcher, console, context, language, SandwichIngredientType::FILLING);
        session.add_ingredients(console, context, std::move(fillings));
        pbf_press_button(context, BUTTON_PLUS, 20, 230);
    }

    {
        IngredientSession session(dispatcher, console, context, language, SandwichIngredientType::CONDIMENT);
        // If there are herbs, we search first from bottom
        if (std::any_of(condiments.begin(), condiments.end(), [&](const auto& p){return p.first.find("herba") != std::string::npos;})){
            pbf_press_dpad(context, DPAD_UP, 20, 105);
        }
        session.add_ingredients(console, context, std::move(condiments));
        pbf_press_button(context, BUTTON_PLUS, 20, 230);
    }

    pbf_mash_button(context, BUTTON_A, 125);
}






}
}
}
