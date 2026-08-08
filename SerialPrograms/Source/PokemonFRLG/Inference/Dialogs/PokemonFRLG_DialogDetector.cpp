/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
//#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
//#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonFRLG_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

WhiteDialogDetector::WhiteDialogDetector(Color color)
    : m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
{}
void WhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool WhiteDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
    ){
        return true;
    }
    return false;
}

AdvanceWhiteDialogDetector::AdvanceWhiteDialogDetector(Color color)
    : m_dialog_box(0.0630769, 0.750154, 0.870154, 0.200423)
    , m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
{}
void AdvanceWhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool AdvanceWhiteDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(game_screen, m_dialog_box),
        combine_rgb(164, 0, 0), combine_rgb(255, 114, 87), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);

    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
        && (stats.average.r > stats.average.b + 180)
        && (stats.average.r > stats.average.g + 180)
    ){
        return true;
    }
    return false;
}

SelectionDialogDetector::SelectionDialogDetector(Color color)
    : m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
    , m_selection_box(0.885231, 0.447962, 0.016, 0.2025)
{}
void SelectionDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_selection_box));
}
bool SelectionDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    ImageViewRGB32 selection_image = extract_box_reference(game_screen, m_selection_box);
    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
        && is_white(selection_image)
    ){
        return true;
    }
    return false;
}

class Test_AdvanceWhiteDialogDetector : public UnitTest{
public:

    Test_AdvanceWhiteDialogDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonFRLG::AdvanceWhiteDialogDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        AdvanceWhiteDialogDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_AdvanceWhiteDialogDetector(UnitTestDatabase& database){
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Eng-Select_False.png", false);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/English-Bulba_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Ger-KennyExile-Bulba_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Ger-KennyExile-Char_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Ger-KennyExile-Sq_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/MacOS-not-ili-Char_False.png", false);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/MacOS-not-ili-Char_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Spanish-AlejaKaiser-Bulba_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Spanish-AlejaKaiser-Char_True.png", true);
    database.add<Test_AdvanceWhiteDialogDetector>("PokemonFRLG/AdvanceWhiteDialogDetector/Spanish-AlejaKaiser-Squirtl_True.png", true);
}

class Test_SelectionDialogDetector : public UnitTest{
public:

    Test_SelectionDialogDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonFRLG::SelectionDialogDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        SelectionDialogDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_SelectionDialogDetector(UnitTestDatabase& database){
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/Eng-Select_True.png", true);
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/English-Bulba-720p_True.png", true);
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/English-Bulba_False.png", false);
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/Ger-KennyExile-Sq_False.png", false);
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/MacOS-not-ili-Char_False.png", false);
    database.add<Test_SelectionDialogDetector>("PokemonFRLG/SelectionDialogDetector/MacOS-not-ili-Char_True.png", true);
}

}
}
}
