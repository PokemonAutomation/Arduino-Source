/*  Common Framework Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/VisualDetectors/BlackBorderDetector.h"
#include "CommonFramework_Tests.h"
#include "TestUtils.h"


//#include <iostream>
//using std::cout;
//using std::cerr;
//using std::endl;

namespace PokemonAutomation{

int test_CommonFramework_BlackBorderDetector(const ImageViewRGB32& image, bool target){
    BlackBorderDetector detector;

    bool result = detector.detect(image);

    TEST_RESULT_EQUAL(result, target);

    return 0;
}


}
