/*  Box Cell Occupied Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BoxCellOccupiedDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

BoxCellOccupiedDetector::BoxCellOccupiedDetector(Color color)
    : m_color(color)
    , m_box(0.007, 0.556, 0.296, 0.253)
{}

void BoxCellOccupiedDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, GameSettings::instance().GAME_BOX.inner_to_outer(m_box));
}

bool BoxCellOccupiedDetector::detect(const ImageViewRGB32& screen){
    const ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    return image_stats(extract_box_reference(game_screen, m_box)).stddev.sum() > 10.0;
}

class Test_BoxCellOccupiedDetector : public UnitTest{
public:
    Test_BoxCellOccupiedDetector(const std::string& image, bool expected)
        : UnitTest("PokemonFRLG::BoxCellOccupiedDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        BoxCellOccupiedDetector detector;
        const bool result = detector.detect(image);
        if (result == m_expected){
            return true;
        }
        return m_expected ? "Expected an occupied cell, received empty." : "Expected an empty cell, received occupied.";
    }

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_BoxCellOccupiedDetector(UnitTestDatabase& database){
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Arcanine_M_100_White.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_Closed_Gold.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_Closed_White.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_YUYV.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_White_HD60Pro_NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxHand_White_HD60Pro_YUYV.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxSelection_Gold.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxSelection_Gold2.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxSelection_White.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/BoxSelection_White2.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Caterpie_F_4_White.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/CloseSelection_Gold.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/CloseSelection_White.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Diglett_F_20_White.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Doduo-NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Doduo-YUYV.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Dragonite_F_100_Whtie.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Nidokinq_M_23_White.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-f-mjpeg.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-f-NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-f-YUYV.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-m-mjpeg.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-m-NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/nidoran-m-YUYV.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/PartySelection_Gold.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/PartySelection_White.png", false);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Rhydon-NV12.png", true);
    database.add<Test_BoxCellOccupiedDetector>("PokemonFRLG/Box/Rhydon-YUYV.png", true);
}

}
}
}
