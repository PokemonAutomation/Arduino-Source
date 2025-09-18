/*  Ingredient Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_IngredientSession_H
#define PokemonAutomation_PokemonSV_IngredientSession_H

#include <map>
#include <memory>
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class SandwichIngredientReader;




struct PageIngredients{
    // the line index of the current selected ingredient, < INGREDIENT_PAGE_LINES
    int8_t selected = -1;
    std::set<std::string> item[SandwichIngredientReader::INGREDIENT_PAGE_LINES];
};


class IngredientSession{
public:
    ~IngredientSession();
    IngredientSession(
        VideoStream& stream, ProControllerContext& context,
        Language language, SandwichIngredientType type
    );

    //  Move to any ingredient in the set. Returns the ingredient it moved to.
    //  Returns empty string if not found.
    std::string move_to_ingredient(const std::set<std::string>& ingredients) const;

    void add_ingredients(
        VideoStream& stream, ProControllerContext& context,
        std::map<std::string, uint8_t>&& ingredients
    );


public:
    PageIngredients read_screen(std::shared_ptr<const ImageRGB32> screenshot) const;
    PageIngredients read_current_page() const;
    bool run_move_iteration(
        std::string& slug, const std::set<std::string>& ingredients,
        const PageIngredients& page
    ) const;


private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    Language m_language;
    VideoOverlaySet m_overlays;
    SandwichIngredientType m_type;
    int8_t m_num_confirmed;
    GradientArrowDetector m_arrow;
};



//  Starting from the top of the fillings menu, gather all the ingredients.
//  When this function returns, the game will be entering the phase where the
//  user must stack the fillings.
//  If any ingredient is not found or insuffient, it will OperationFailedException::fire.
void add_sandwich_ingredients(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>&& fillings,  //  {slug, quantity}
    std::map<std::string, uint8_t>&& condiments
);




}
}
}
#endif
