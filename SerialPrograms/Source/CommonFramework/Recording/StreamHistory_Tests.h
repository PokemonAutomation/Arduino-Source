/*  Stream History Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StreamHistory_Tests_H
#define PokemonAutomation_StreamHistory_Tests_H

namespace PokemonAutomation{

class UnitTestDatabase;



//
//  Registers the stream history tests: the H.264 bitstream helpers used when
//  muxing the ring into an MP4 (Annex B to AVCC packets, and the avcC record
//  built from Annex B extradata). Pure data tests: no ffmpeg, no files.
//
void add_tests_StreamHistory(UnitTestDatabase& database);



}
#endif
