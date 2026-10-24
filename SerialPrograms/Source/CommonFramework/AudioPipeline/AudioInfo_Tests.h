/*  Audio Pipeline - AudioInfo / Input Choice Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioInfo_Tests_H
#define PokemonAutomation_AudioPipeline_AudioInfo_Tests_H

class UnitTestDatabase;

namespace PokemonAutomation{



//
//  Registers the tests for AudioInputChoice / input_choices(): the
//  device-driven input format list (display naming, dedup / sort, the
//  1ch x 96000 misreporting capture-card reinterpretation, the
//  preferred-index rules, and choice equality).
//
void add_tests_AudioInfo(UnitTestDatabase& database);



}
#endif
