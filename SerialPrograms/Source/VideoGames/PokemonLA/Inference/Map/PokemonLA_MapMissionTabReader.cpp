/*  Map Zoom Level Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/SolidColorTest.h"
//#include "Tests/TestUtils.h"
#include "Tests/ImageUnitTest.h"
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









class Test_MapMissionTabReader : public ImageUnitTest{
public:
    Test_MapMissionTabReader(
        const std::string& image,
        bool expected
    )
        : ImageUnitTest("PokemonLA::MapMissionTabReader - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];
        return is_map_mission_tab_raised(image) == m_expected;
    };

private:
    bool m_expected;
};



void add_tests_MapMissionTabReader(UnitTestDatabase& database){

}





}
}
}
