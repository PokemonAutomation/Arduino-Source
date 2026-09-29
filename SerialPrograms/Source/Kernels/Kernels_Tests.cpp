/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels_Tests.h"
#include "BinaryMatrix/Kernels_BinaryMatrix_Tests.h"
#include "ImageFilters/Kernels_ImageFilter_Tests.h"
#include "ImageScaleBrightness/Kernels_ImageScaleBrightness_Tests.h"
#include "Waterfill/Kernels_Waterfill_Tests.h"

namespace PokemonAutomation{
namespace Kernels{



void add_tests(UnitTestDatabase& database){
    add_tests_BinaryMatrix(database);
    add_tests_ImageFilters(database);
    add_tests_ImageScaleBrightness(database);
    add_tests_Waterfill(database);
}



}
}
