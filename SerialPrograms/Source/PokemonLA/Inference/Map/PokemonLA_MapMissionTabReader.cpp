/*  Map Zoom Level Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/TestUtils.h"
#include "PokemonLA_MapMissionTabReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

bool is_map_mission_tab_raised(const ImageViewRGB32& screen){
    // The white area around the "R" button when the tab is raise.
    const ImageFloatBox box0{0.9235, 0.617, 0.003, 0.019};
    const ImageFloatBox box1{0.937, 0.62, 0.0035, 0.012};

    return is_white(image_stats(extract_box_reference(screen, box0))) || 
        is_white(image_stats(extract_box_reference(screen, box1)));
}









class Test_MapMissionTabReader : public UnitTest{
public:
    Test_MapMissionTabReader(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonLA::MapMissionTabReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        return is_map_mission_tab_raised(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};



void add_tests_MapMissionTabReader(UnitTestDatabase& database){

}





}
}
}
