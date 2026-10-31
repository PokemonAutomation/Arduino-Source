/*  Bag Detector Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "PokemonFRLG_BagDetector.h"
#include "PokemonFRLG_BagDetector_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

class Test_BagDetector : public UnitTest{
public:
    Test_BagDetector(const std::string& image, bool expected)
        : UnitTest("PokemonFRLG::BagDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        BagDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        const bool result = detector.detect(image);
        if (result != m_expected){
            return std::string("Expected bag = ") + (m_expected ? "true" : "false") +
                ", got " + (result ? "true" : "false");
        }
        return true;
    }

private:
    std::string m_image;
    bool m_expected;
};

}


void add_tests_BagDetector(UnitTestDatabase& database){
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Top.png", true);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/KeyItems_Top.png", true);
    //  Scrolled down, so a red up-arrow covers the middle of the top frame.
    //  Fails with the current detector: its top-frame strip runs under the arrow.
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Items_Scrolled.png", true);

    database.add<Test_BagDetector>("PokemonFRLG/Party/Party_1.png", false);
    database.add<Test_BagDetector>("PokemonFRLG/Box/BoxSelection_White.png", false);
    database.add<Test_BagDetector>("PokemonFRLG/Bag/Overworld_Route24.png", false);
}



}
}
}
