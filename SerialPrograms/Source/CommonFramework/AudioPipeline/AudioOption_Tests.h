/*  Audio Pipeline - AudioOption JSON Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioOption_Tests_H
#define PokemonAutomation_AudioPipeline_AudioOption_Tests_H

class UnitTestDatabase;

namespace PokemonAutomation{



//
//  Registers the tests for the descriptor-based AudioOption JSON layout
//  (SourceType + type-keyed params node, mirroring VideoSourceOption).
//
void add_tests_AudioOption(UnitTestDatabase& database);



}
#endif
