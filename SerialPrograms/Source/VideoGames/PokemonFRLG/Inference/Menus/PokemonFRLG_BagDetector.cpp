/*  Bag Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonFRLG_BagDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


BagDetector::BagDetector(Color color)
    : m_bag_shadow_box(0.131250, 0.608173, 0.068269, 0.001442)
    , m_pocket_underline_box(0.043269, 0.137259, 0.279808, 0.005769)
    //  Stops short of the red arrow drawn over the frame once the list is
    //  scrolled down.
    , m_menu_top_box(0.029808, 0.029086, 0.550000, 0.003606)
    , m_menu_right_box(0.977404, 0.034134, 0.005288, 0.597837)
{}
void BagDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bag_shadow_box));        // Gray   RGB(159, 159, 159)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_pocket_underline_box));  // Orange RGB(255, 152,  50)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));          // Yellow RGB(255, 225,  93)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_box));        // Yellow

}
bool BagDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 bag_shadow_image = extract_box_reference(game_screen, m_bag_shadow_box);
    ImageViewRGB32 pocket_underline_image = extract_box_reference(game_screen, m_pocket_underline_box);
    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_right_image = extract_box_reference(game_screen, m_menu_right_box);


    if (   is_solid(bag_shadow_image,       { 0.333, 0.333, 0.333 }, 0.25, 20)
        && is_solid(pocket_underline_image, { 0.558, 0.333, 0.109 }, 0.25, 20)
        && is_solid(menu_top_image,         { 0.445, 0.393, 0.162 }, 0.25, 20)
        && is_solid(menu_right_image,       { 0.445, 0.393, 0.162 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}


class Test_BagDetector : public ImageUnitTest{
public:
    Test_BagDetector(const std::string& image, bool expected)
        : ImageUnitTest("PokemonFRLG::BagDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        BagDetector detector(COLOR_RED);
        const ImageRGB32& image = images[0];
        const bool result = detector.detect(image);
        if (result != m_expected){
            return std::string("Expected bag = ") + (m_expected ? "true" : "false") +
                ", got " + (result ? "true" : "false");
        }
        return true;
    }

private:
    bool m_expected;
};

void add_tests_BagDetector(UnitTestDatabase& database){
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Top_True.png", true);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/KeyItems_Top_True.png", true);
    //  Scrolled down: a red up-arrow covers the middle of the top frame.
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Scrolled_True.png", true);

    //  A girl's bag, drawn differently from the boy's.
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Top_Girl_True.png", true);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Potion_Girl_True.png", true);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/KeyItems_Top_Girl_True.png", true);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/PokeBalls_Top_Girl_True.png", true);

    database.add<Test_BagDetector>("PokemonFRLG/Bag/Overworld_Route24_False.png", false);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Overworld_Bedroom_Girl_False.png", false);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/StartMenu_Girl_False.png", false);
}



}
}
}
