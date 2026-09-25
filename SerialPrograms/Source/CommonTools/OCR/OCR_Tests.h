/*  OCR Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OCR_Tests_H
#define PokemonAutomation_OCR_Tests_H

#include "Common/Cpp/TestRunners/UnitTest.h"

namespace PokemonAutomation{
namespace OCR{



void add_tests(UnitTestDatabase& database);

void add_tests_raw_OCR(UnitTestDatabase& database);

void add_tests_number_waterfill_OCR(UnitTestDatabase& database);

// Multi-line OCR tests. These exercise the PaddleOCR detection + recognition pipeline
// through `ocr_read()` with a multi-line page segmentation mode.
void add_tests_multiline_OCR(UnitTestDatabase& database);



}
}
#endif
