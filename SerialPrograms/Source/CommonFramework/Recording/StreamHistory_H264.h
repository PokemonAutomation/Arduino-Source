/*  Stream History - H.264 Bitstream Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Conversions between the two H.264 bitstream forms, used when muxing the
 *  stream history ring (StreamHistoryTracker_Ring) into an MP4 file. Pure
 *  byte-buffer functions: no ffmpeg or Qt.
 *
 */

#ifndef PokemonAutomation_StreamHistory_H264_H
#define PokemonAutomation_StreamHistory_H264_H

#include <stddef.h>
#include <stdint.h>
#include <vector>

namespace PokemonAutomation{


// Converts H.264 video data from Annex B format to AVCC format.
// Returns false if no NAL unit is found.
//
// In H.264 video streaming, individual packets of video data are called NAL 
// (Network Abstraction Layer) units. 
// • Annex B (Input): Separates NAL units using a "Start Code" delimiter 
// (usually 0x00 0x00 0x01 or 0x00 0x00 0x00 0x01).
// • AVCC (Output): Prefixes each NAL unit with a fixed-size length field
// (usually 4 bytes) specifying exactly how many bytes follow. This is
// commonly used in file containers like MP4, MKV, and FLV because it allows
// the player to quickly skip from one packet to the next without scanning the data.
// 
// The hardware encoders (e.g. h264_amf, qsv, videotoolbox) produce video in the Annex
// B format, so they must be converted to avcc, which is what the muxer expects.
bool h264_annexb_to_avcc(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out);

//  Builds the MP4 avcC configuration record from an Annex B H.264 extradata
//  (SPS/PPS NAL units with start codes). Returns false if no SPS is found.
bool h264_avcc_from_annexb_extradata(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out);


}
#endif
