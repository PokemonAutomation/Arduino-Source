/*  Stream History - H.264 Bitstream Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "StreamHistory_H264.h"

namespace PokemonAutomation{


namespace{


//  The first 3-byte start code (00 00 01) in [p, end), or end if there is
//  none. This also finds a 4-byte start code (00 00 00 01): it points at its
//  last three bytes.
const uint8_t* find_start_code(const uint8_t* p, const uint8_t* end){
    for (; p + 2 < end; p++){
        if (p[0] == 0 && p[1] == 0 && p[2] == 1){
            return p;
        }
    }
    return end;
}

//  Calls nal(start, size) for each non-empty NAL unit of an Annex B buffer,
//  in order. Bytes before the first start code are skipped. Empty NAL units
//  (two start codes in a row) are skipped too.
template <typename Callback>
void for_each_annexb_nal(const uint8_t* in, size_t in_size, Callback&& nal){
    const uint8_t* end = in + in_size;
    const uint8_t* start_code = find_start_code(in, end);
    while (start_code != end){
        const uint8_t* nal_start = start_code + 3;
        const uint8_t* next = find_start_code(nal_start, end);
        //  A NAL unit never ends in a zero byte, so trailing zeros belong to
        //  the next start code (the leading zero of 00 00 00 01) or are
        //  trailing_zero_8bits padding, not to the NAL unit.
        const uint8_t* nal_end = next;
        while (nal_end > nal_start && nal_end[-1] == 0){
            nal_end--;
        }
        if (nal_end > nal_start){
            nal(nal_start, (size_t)(nal_end - nal_start));
        }
        start_code = next;
    }
}


//  Reads the exp-Golomb coded fields of a NAL unit's payload (RBSP).
class RbspReader{
public:
    //  'nal' is the NAL unit after its 1-byte header. Removes the emulation
    //  prevention bytes (the 03 in 00 00 03).
    RbspReader(const uint8_t* nal, size_t size){
        size_t zeros = 0;
        for (size_t i = 0; i < size; i++){
            if (zeros >= 2 && nal[i] == 3){
                zeros = 0;
                continue;
            }
            zeros = nal[i] == 0 ? zeros + 1 : 0;
            m_rbsp.push_back(nal[i]);
        }
    }

    bool read_bit(uint32_t& bit){
        if (m_bit >= m_rbsp.size() * 8){
            return false;
        }
        bit = (m_rbsp[m_bit / 8] >> (7 - m_bit % 8)) & 1;
        m_bit++;
        return true;
    }
    bool read_bits(size_t count, uint32_t& value){
        value = 0;
        for (size_t i = 0; i < count; i++){
            uint32_t bit;
            if (!read_bit(bit)){
                return false;
            }
            value = (value << 1) | bit;
        }
        return true;
    }
    //  ue(v): unsigned exp-Golomb.
    bool read_ue(uint32_t& value){
        size_t leading_zeros = 0;
        while (true){
            uint32_t bit;
            if (!read_bit(bit)){
                return false;
            }
            if (bit != 0){
                break;
            }
            leading_zeros++;
            if (leading_zeros > 31){
                return false;
            }
        }
        uint32_t suffix;
        if (!read_bits(leading_zeros, suffix)){
            return false;
        }
        value = (uint32_t)((1ull << leading_zeros) - 1) + suffix;
        return true;
    }

private:
    std::vector<uint8_t> m_rbsp;
    size_t m_bit = 0;
};


//  The SPS fields of the avcC High profile extension.
struct SpsFormat{
    uint32_t chroma_format_idc = 1;
    uint32_t bit_depth_luma_minus8 = 0;
    uint32_t bit_depth_chroma_minus8 = 0;
};

//  The profiles whose SPS carries chroma_format_idc and the bit depths
//  (H.264 7.3.2.1.1). The other profiles are 4:2:0 at 8 bits.
bool sps_has_format_fields(uint8_t profile_idc){
    switch (profile_idc){
    case 100: case 110: case 122: case 244: case 44:
    case 83: case 86: case 118: case 128: case 138:
    case 139: case 134: case 135:
        return true;
    default:
        return false;
    }
}

//  Reads the chroma format and bit depths from an SPS NAL unit (header
//  included, at least 4 bytes). Returns false if the SPS is cut short.
bool parse_sps_format(const std::vector<uint8_t>& sps, SpsFormat& format){
    format = SpsFormat();
    uint8_t profile_idc = sps[1];
    if (!sps_has_format_fields(profile_idc)){
        return true;
    }
    //  After the header: profile_idc, constraint flags, level_idc, then the
    //  exp-Golomb fields.
    RbspReader reader(sps.data() + 1, sps.size() - 1);
    uint32_t skipped;
    if (!reader.read_bits(24, skipped)){
        return false;
    }
    uint32_t seq_parameter_set_id;
    if (!reader.read_ue(seq_parameter_set_id) || !reader.read_ue(format.chroma_format_idc)){
        return false;
    }
    if (format.chroma_format_idc == 3){
        uint32_t separate_colour_plane_flag;
        if (!reader.read_bit(separate_colour_plane_flag)){
            return false;
        }
    }
    return reader.read_ue(format.bit_depth_luma_minus8)
        && reader.read_ue(format.bit_depth_chroma_minus8)
        && format.chroma_format_idc <= 3
        && format.bit_depth_luma_minus8 <= 6
        && format.bit_depth_chroma_minus8 <= 6;
}


}



bool h264_annexb_to_avcc(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    bool found_any = false;
    for_each_annexb_nal(in, in_size, [&](const uint8_t* nal, size_t size){
        found_any = true;
        out.push_back((uint8_t)(size >> 24));
        out.push_back((uint8_t)(size >> 16));
        out.push_back((uint8_t)(size >> 8));
        out.push_back((uint8_t)(size));
        out.insert(out.end(), nal, nal + size);
    });
    return found_any;
}

bool h264_avcc_from_annexb_extradata(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    for_each_annexb_nal(in, in_size, [&](const uint8_t* nal, size_t size){
        int type = nal[0] & 0x1F;
        if (type == 7 && sps.empty()){
            sps.assign(nal, nal + size);    //  SPS NAL unit.
        }else if (type == 8 && pps.empty()){
            pps.assign(nal, nal + size);    //  PPS NAL unit.
        }
    });
    if (sps.size() < 4){
        return false;
    }
    SpsFormat format;
    if (!parse_sps_format(sps, format)){
        return false;
    }

    //  avcC (ISO/IEC 14496-15): configurationVersion=1, profile_idc,
    //  profile_compatibility, level_idc, 6 reserved bits + lengthSizeMinusOne
    //  (3 => 4-byte NAL unit lengths), 3 reserved bits +
    //  numOfSequenceParameterSets (1), then the SPS with its 16-bit length,
    //  numOfPictureParameterSets (1, or 0 if the extradata has no PPS), and
    //  the PPS with its 16-bit length.
    out.push_back(0x01);
    out.push_back(sps[1]);
    out.push_back(sps[2]);
    out.push_back(sps[3]);
    out.push_back(0xFF);
    out.push_back(0xE1);
    out.push_back((uint8_t)(sps.size() >> 8));
    out.push_back((uint8_t)(sps.size() & 0xFF));
    out.insert(out.end(), sps.begin(), sps.end());
    if (pps.empty()){
        out.push_back(0x00);
    }else{
        out.push_back(0x01);
        out.push_back((uint8_t)(pps.size() >> 8));
        out.push_back((uint8_t)(pps.size() & 0xFF));
        out.insert(out.end(), pps.begin(), pps.end());
    }

    //  Every profile but Baseline (66), Main (77) and Extended (88) ends the
    //  record with the chroma format and bit depths (6 and 5 reserved bits
    //  set), and the count of SPS extensions (none).
    uint8_t profile_idc = sps[1];
    if (profile_idc != 66 && profile_idc != 77 && profile_idc != 88){
        out.push_back((uint8_t)(0xFC | format.chroma_format_idc));
        out.push_back((uint8_t)(0xF8 | format.bit_depth_luma_minus8));
        out.push_back((uint8_t)(0xF8 | format.bit_depth_chroma_minus8));
        out.push_back(0x00);
    }
    return true;
}


}
