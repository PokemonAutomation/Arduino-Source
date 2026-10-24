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
//  Registers the sink gate tests: self-contained invariants for the
//  pipeline tree's sink branch (RawAudio2WorkingAudio -> [resampler] ->
//  [mixer] -> WorkingAudio2RawAudio) - no-op bit-exactness, 1-LSB sint16
//  against a plain-scalar reference, and the resampled / mixed shapes
//  (output length, tone amplitudes, upmix / downmix channel behavior).
//
void add_tests_SinkPipeline(UnitTestDatabase& database);



}
#endif
