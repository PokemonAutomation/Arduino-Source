/*  Audio Pipeline - Audio File Loader Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioFileLoader_Tests_H
#define PokemonAutomation_AudioPipeline_AudioFileLoader_Tests_H

class UnitTestDatabase;

namespace PokemonAutomation{



//
//  Registers the WAV-native file input tests: WavFile header parsing
//  (PCM 8/16/32-bit and IEEE-float), probe_wav_native_format(), and
//  AudioFileLoader::load_full_audio()'s batch conversion. The test WAVs are
//  generated on the fly in the system temp directory; no audio I/O is
//  involved.
//
void add_tests_AudioFileLoader(UnitTestDatabase& database);



}
#endif
