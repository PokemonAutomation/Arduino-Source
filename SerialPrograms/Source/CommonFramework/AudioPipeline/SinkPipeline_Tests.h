/*  Audio Pipeline - Sink Gate Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_SinkPipeline_Tests_H
#define PokemonAutomation_AudioPipeline_SinkPipeline_Tests_H

class UnitTestDatabase;

namespace PokemonAutomation{



//
//  Registers the sink gate tests: raw-byte comparisons between the
//  pipeline tree's sink branch (RawAudio2WorkingAudio -> [resampler] ->
//  [mixer] -> WorkingAudio2RawAudio) and the legacy converter chain
//  (AudioStreamToFloat -> AudioFloatToStream), plus self-contained
//  invariants for the resampled / mixed shapes the legacy path never
//  produced (it opened the device in the input's format instead).
//
void add_tests_SinkPipeline(UnitTestDatabase& database);



}
#endif
