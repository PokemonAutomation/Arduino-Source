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
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class SandwichIngredientReader;


// Detection output of one ingredient menu page: cursor position and detected ingredients per line.
// Returned by IngredientSession::read_screen()/read_current_page()
struct PageIngredients{
    // Line index (0-9) of cursor-selected ingredient. -1 if detection failed.
    int8_t selected = -1;

    // Detected ingredient slugs for each line (At most INGREDIENT_PAGE_LINES lines). Sets contain
    // multiple slugs if OCR is ambiguous.
    std::set<std::string> item[SandwichIngredientReader::INGREDIENT_PAGE_LINES];
};


// Manages navigation and selection of ingredients in the sandwich menu (fillings or condiments).
// Uses OCR+sprite matching to detect ingredients, navigates cursor, and confirms selections.
// It's only used in add_sandwich_ingredients() function.
class IngredientSession{
public:
    ~IngredientSession();

    IngredientSession(
        VideoStream& stream, ProControllerContext& context,
        Language language, SandwichIngredientType type
    );

    // Adds ingredients to sandwich by navigating menu, pressing A, and verifying each addition.
    // Throws if ingredient not found or failed to verify the addition.
    // Example: {{"lettuce", 2}, {"tomato", 1}} adds 2 lettuce and 1 tomato.
    void add_ingredients(
        VideoStream& stream, ProControllerContext& context,
        std::map<std::string, uint8_t>&& ingredients
    );

private:
    // Navigates menu to find and select any ingredient from the set. Scrolls pages as needed.
    // Returns ingredient slug if found, empty string if not found after checking all pages.
    std::string move_to_ingredient(const std::map<std::string, uint8_t>& ingredients) const;

    // Reads screenshot with OCR+sprite matching. Detects cursor position and all visible ingredients.
    // Uses dual detection (OCR + sprite) on selected item to remove OCR ambiguity.
    PageIngredients read_screen(std::shared_ptr<const ImageRGB32> screenshot) const;

    // Captures current frame and calls read_screen().
    PageIngredients read_current_page() const;

    // Checks if any desired ingredient is on the current page.
    // If no desired ingredients found, returns false and does not change `slug`.
    // If at least one desired ingredients is found:
    // - If the current cursor is already on a desired ingredient, returns True and sets
    //   `slug` to the ingredient slug.
    // - Otherwise, there are at least one desired ingredient at least one line away from
    //   the cursor. Move the cursor blindly towards the closest ingredient, returns True
    //   and does not change `slug`.
    bool run_move_iteration(
        std::string& slug, const std::map<std::string, uint8_t>& ingredients,
        const PageIngredients& page
    ) const;


private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    Language m_language;
    VideoOverlaySet m_overlays;
    SandwichIngredientType m_type; // FILLING or CONDIMENT (which menu type)
    int8_t m_num_confirmed; // Running count of confirmed ingredient additions
    GradientArrowDetector m_arrow; // Detects the cursor arrow position
};



// Adds all fillings and condiments to a sandwich.
// Navigates both menus, adds ingredients, confirms additions, and mashes A in the end to select the first
// pick and go to the ingredient stacking mini-game.
// Note: There is no wait for the microcontroller commands in the final button mashing phase.
// Throws OperationFailedException if any ingredient not found or addition fails.
void add_sandwich_ingredients(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    std::map<std::string, uint8_t>&& fillings,     // {slug, quantity} e.g. {{"lettuce", 2}}
    std::map<std::string, uint8_t>&& condiments    // {slug, quantity} e.g. {{"salt", 1}}
);




}
}
}
#endif
