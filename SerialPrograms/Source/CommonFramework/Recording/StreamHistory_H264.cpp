/*  Stream History - H.264 Bitstream Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "StreamHistory_H264.h"

namespace PokemonAutomation{


bool h264_annexb_to_avcc(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    const uint8_t* p = in;
    const uint8_t* end = in + in_size;
    bool found_any = false;
    while (p < end){
        // find start of NAL unit. 
        // Looks for 3-byte Annex B start code: 0x00 0x00 0x01.
        // this will also find the 4-byte start code: 00 00 00 01
        const uint8_t* nal_start = nullptr;
        for (const uint8_t* q = p; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_start = q + 3;
                break;
            }
        }
        if (nal_start == nullptr){
            break;    //  No start code left; stop at any trailing garbage.
        }

        // find end of NAL unit
        // scan forward until  you find the next start code
        // if don't find another start code, assume this is the final packet
        const uint8_t* nal_end = nal_start;
        for (const uint8_t* q = nal_start; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_end = q;
                break;
            }
        }
        if (nal_end == nal_start){
            nal_end = end;    //  The last NAL unit runs to the end of the buffer.
        }
        size_t nal_size = (size_t)(nal_end - nal_start);
        if (nal_size == 0){
            break;
        }
        found_any = true;
        out.push_back((uint8_t)(nal_size >> 24));
        out.push_back((uint8_t)(nal_size >> 16));
        out.push_back((uint8_t)(nal_size >> 8));
        out.push_back((uint8_t)(nal_size));
        out.insert(out.end(), nal_start, nal_end);
        p = nal_end;
    }
    return found_any;
}

bool h264_avcc_from_annexb_extradata(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    const uint8_t* p = in;
    const uint8_t* end = in + in_size;
    while (p < end){
        const uint8_t* nal_start = nullptr;
        for (const uint8_t* q = p; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_start = q + 3;
                break;
            }
        }
        if (nal_start == nullptr){
            break;
        }
        const uint8_t* nal_end = nal_start;
        for (const uint8_t* q = nal_start; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_end = q;
                break;
            }
        }
        if (nal_end == nal_start){
            nal_end = end;
        }
        if (nal_start < nal_end){
            int type = nal_start[0] & 0x1F;
            if (type == 7 && sps.empty()){
                sps.assign(nal_start, nal_end);    //  SPS NAL unit.
            }else if (type == 8 && pps.empty()){
                pps.assign(nal_start, nal_end);    //  PPS NAL unit.
            }
        }
        p = nal_end;
    }
    if (sps.size() < 4){
        return false;
    }
    //  avcC: configurationVersion=1, profile_idc, profile_compatibility,
    //  level_idc, 6 reserved bits + lengthSizeMinusOne (3 => 4-byte NAL
    //  unit lengths), 3 reserved bits + numOfSequenceParameterSets (1),
    //  then the SPS with its 16-bit length, numOfPictureParameterSets (1),
    //  and the PPS with its 16-bit length.
    out.push_back(0x01);
    out.push_back(sps[1]);
    out.push_back(sps[2]);
    out.push_back(sps[3]);
    out.push_back(0xFF);
    out.push_back(0xE1);
    out.push_back((uint8_t)(sps.size() >> 8));
    out.push_back((uint8_t)(sps.size() & 0xFF));
    out.insert(out.end(), sps.begin(), sps.end());
    out.push_back(0x01);
    out.push_back((uint8_t)(pps.size() >> 8));
    out.push_back((uint8_t)(pps.size() & 0xFF));
    out.insert(out.end(), pps.begin(), pps.end());
    return true;
}


}
