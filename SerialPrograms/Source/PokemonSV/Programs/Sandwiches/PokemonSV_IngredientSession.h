/*  Ingredient Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_IngredientSession_H
#define PokemonAutomation_PokemonSV_IngredientSession_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"


namespace PokemonAutomation{
    class AsyncDispatcher;
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{

class SandwichIngredientReader;


static constexpr size_t INGREDIENT_PAGE_LINES = 10;

struct PageIngredients{
    int8_t selected = -1;
    std::set<std::string> item[INGREDIENT_PAGE_LINES];
};


class IngredientSession{
public:
    ~IngredientSession();
    IngredientSession(
        AsyncDispatcher& dispatcher,
        ConsoleHandle& console, BotBaseContext& context,
        Language language
    );

    //  Move to any ingredient in the set. Returns the ingredient it moved to.
    //  Returns empty string if not found.
    std::string move_to_ingredient(const std::set<std::string>& ingredients) const;

public:
    PageIngredients read_current_page() const;

    bool run_move_iteration(
        std::string& slug, const std::set<std::string>& ingredients,
        const PageIngredients& page
    ) const;

private:
    AsyncDispatcher& m_dispatcher;
    ConsoleHandle& m_console;
    BotBaseContext& m_context;
    Language m_language;
    VideoOverlaySet m_overlays;
    FixedLimitVector<SandwichIngredientReader> m_lines;
    GradientArrowDetector m_arrow;
};



}
}
}
#endif
