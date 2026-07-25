/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "PokemonSwSh_BoxGenderDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


BoxGenderDetector::BoxGenderDetector(Color color)
    : Pokemon::BoxGenderDetector({0.720, 0.028, 0.229, 0.056}, 0.01f, color)
{}







class Test_BoxGenderDetector : public UnitTest{
public:
    Test_BoxGenderDetector(
        const std::string& image,
        StatsHuntGenderFilter expected
    )
        : UnitTest("PokemonSwSh::BoxGenderDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(CancellableScope& scope) const override{
        BoxGenderDetector detector;
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    StatsHuntGenderFilter m_expected;
};








void add_tests_BoxGenderDetector(UnitTestDatabase& database){
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/macOS_bright/Female_2.png", StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/macOS_bright/Genderless_3.png", StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/macOS_bright/Male_1.png", StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/Switch/Centiskorch_2.jpg", StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/Switch/Claydol_3.jpg", StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/Switch/Nuzleaf_1.jpg", StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/WinMyPin/Gardevoir_2.png", StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/WinNoNameBrand/Gardevoir_2.png", StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/WinNoNameBrand/Thwackey_1.png", StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/WinNoNameBrand/Urshifu_1.png", StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonSwSh/BoxGenderDetector/WinNoNameBrand/Zigzagoon_2.png", StatsHuntGenderFilter::Female);
}












}
}
}

