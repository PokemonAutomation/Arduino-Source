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
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV_IngredientSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

IngredientSession::~IngredientSession() = default;

IngredientSession::IngredientSession(
    AsyncDispatcher& dispatcher,
    ConsoleHandle& console, BotBaseContext& context,
    Language language
)
    : m_dispatcher(dispatcher)
    , m_console(console)
    , m_context(context)
    , m_language(language)
    , m_overlays(console.overlay())
    , m_lines(10)
    , m_arrow(COLOR_CYAN, GradientArrowType::RIGHT, {0.02, 0.15, 0.05, 0.80})
{
    for (size_t c = 0; c < INGREDIENT_PAGE_LINES; c++){
        m_lines.emplace_back(SandwichIngredientType::FILLING, c, COLOR_CYAN);
        m_lines.back().make_overlays(m_overlays);
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
            OCR::StringMatchResult result = m_lines[index].read_with_ocr(snapshot, m_console, m_language);
            result.clear_beyond_log10p(SandwichFillingOCR::MAX_LOG10P);
            result.clear_beyond_spread(SandwichFillingOCR::MAX_LOG10P_SPREAD);
            for (auto& item : result.results){
                ret.item[index].insert(item.second.token);
            }
        }else{
            image_result = m_lines[ret.selected].read_with_icon_matcher(snapshot);
            image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
            image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
            image_result.log(m_console, SandwichIngredientReader::MAX_ALPHA);
        }
    });

#if 0
    //  Now read the sprite for the selected line.
    ImageMatch::ImageMatchResult image_result = m_lines[ret.selected].read_with_icon_matcher(snapshot);
    image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
    image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
    image_result.log(m_console, SandwichIngredientReader::MAX_ALPHA);
#endif

#if 0
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
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Unable to read selected item. OCR and sprite do not agree on any match.",
            snapshot
        );
    }
    if (common.size() > 1){
        throw OperationFailedException(
            ErrorReport::SEND_ERROR_REPORT, m_console,
            "IngredientSession::read_current_page(): Unable to read selected item. Ambiguous result.",
            snapshot
        );
    }

    ocr_result = std::move(common);
#endif

    return ret;
}
bool IngredientSession::run_move_iteration(
    std::string& slug, const std::set<std::string>& ingredients,
    const PageIngredients& page
) const{
    //  Returns true if a desired ingredient is found somewhere on the page.
    //  "slug" is set the desired ingredient if the cursor is already on it.

    for (size_t c = 0; c < INGREDIENT_PAGE_LINES; c++){
        for (const std::string& item : page.item[c]){
            auto iter = ingredients.find(item);
            if (iter == ingredients.end()){
                continue;
            }

            size_t current = page.selected;
            size_t desired = c;

            //  Cursor is already on matching ingredient.
            if (current == desired){
                m_console.log("Desired ingredient is selected!", COLOR_BLUE);
                slug = *iter;
                return true;
            }

            m_console.log("Found desired ingredient. Moving towards it...", COLOR_BLUE);

            //  Move to it.
            while (current < desired){
                pbf_press_dpad(m_context, DPAD_DOWN, 10, 30);
                current++;
            }
            while (current > desired){
                pbf_press_dpad(m_context, DPAD_UP, 10, 30);
                current--;
            }
            m_context.wait_for_all_requests();
            m_context.wait_for(std::chrono::seconds(1));

            return true;
        }
    }

    return false;
}
std::string IngredientSession::move_to_ingredient(const std::set<std::string>& ingredients) const{
    if (ingredients.empty()){
        m_console.log("No desired ingredients.", COLOR_RED);
        return "";
    }

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
            m_console.log("Ingredient not found anywhere.", COLOR_RED);
            return "";
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




}
}
}
