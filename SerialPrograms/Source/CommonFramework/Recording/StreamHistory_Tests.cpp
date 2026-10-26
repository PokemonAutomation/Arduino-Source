/*  Stream History Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tests for the H.264 bitstream helpers (StreamHistory_H264.h). Each case
 *  checks the exact output bytes.
 *
 */

#include <stdint.h>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "StreamHistory_H264.h"
#include "StreamHistory_Tests.h"

namespace PokemonAutomation{

namespace{



using Bytes = std::vector<uint8_t>;

Bytes concat(std::initializer_list<Bytes> parts){
    Bytes ret;
    for (const Bytes& part : parts){
        ret.insert(ret.end(), part.begin(), part.end());
    }
    return ret;
}

std::string to_hex(const Bytes& bytes){
    const char* digits = "0123456789abcdef";
    std::string ret;
    for (size_t i = 0; i < bytes.size(); i++){
        if (i != 0){
            ret += ' ';
        }
        ret += digits[bytes[i] >> 4];
        ret += digits[bytes[i] & 0xF];
    }
    return ret.empty() ? "(empty)" : ret;
}

//  Runs one helper and compares its return value and output bytes. The
//  output vector starts out non-empty, so the helper must replace it rather
//  than append to it.
template <typename Function>
UnitTestResult check_conversion(
    Function function,
    const Bytes& input,
    bool expected_ok,
    const Bytes& expected
){
    Bytes out{0xee, 0xee};
    bool ok = function(input.data(), input.size(), out);
    if (ok != expected_ok){
        return std::string("Returned ") + (ok ? "true" : "false")
            + ", expected " + (expected_ok ? "true" : "false")
            + ". Output: " + to_hex(out);
    }
    if (!expected_ok){
        return true;
    }
    if (out != expected){
        return "Output: " + to_hex(out) + "\nExpected: " + to_hex(expected);
    }
    return true;
}


const Bytes START3{0x00, 0x00, 0x01};
const Bytes START4{0x00, 0x00, 0x00, 0x01};

//  Sample parameter sets (High profile, level 3.1) and NAL units.
const Bytes SPS{0x67, 0x64, 0x00, 0x1f, 0xac, 0xd9, 0x40};
const Bytes PPS{0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0};
const Bytes SEI{0x06, 0x05, 0xff};
const Bytes AUD{0x09, 0xf0};
const Bytes IDR{0x65, 0x88, 0x84};

//  The avcC record for SPS + PPS: version 1, profile/compatibility/level
//  from SPS bytes 1-3, 4-byte NAL lengths, one SPS, one PPS.
const Bytes SPS_PPS_AVCC = concat({
    {0x01, 0x64, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x07},
    SPS,
    {0x01, 0x00, 0x06},
    PPS,
});



class AnnexBToAvccTest : public UnitTest{
public:
    AnnexBToAvccTest(const std::string& name, Bytes input, bool expected_ok, Bytes expected)
        : UnitTest("StreamHistory - H264 AnnexB to AVCC - " + name)
        , m_input(std::move(input))
        , m_expected_ok(expected_ok)
        , m_expected(std::move(expected))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        return check_conversion(h264_annexb_to_avcc, m_input, m_expected_ok, m_expected);
    }

private:
    Bytes m_input;
    bool m_expected_ok;
    Bytes m_expected;
};


class AvccFromExtradataTest : public UnitTest{
public:
    AvccFromExtradataTest(const std::string& name, Bytes input, bool expected_ok, Bytes expected)
        : UnitTest("StreamHistory - H264 avcC from extradata - " + name)
        , m_input(std::move(input))
        , m_expected_ok(expected_ok)
        , m_expected(std::move(expected))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        return check_conversion(h264_avcc_from_annexb_extradata, m_input, m_expected_ok, m_expected);
    }

private:
    Bytes m_input;
    bool m_expected_ok;
    Bytes m_expected;
};



}



void add_tests_StreamHistory(UnitTestDatabase& database){
    //  Annex B packets to AVCC (4-byte big-endian length prefixes).
    database.add<AnnexBToAvccTest>(
        "4-byte start code",
        concat({START4, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "3-byte start code",
        concat({START3, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "Several NALs, 3-byte separators",
        concat({START4, AUD, START3, SPS, START3, PPS, START3, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x07}, SPS,
            {0x00, 0x00, 0x00, 0x06}, PPS,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    //  A NAL unit never ends in a zero byte, so the zero in front of a
    //  following 4-byte start code is part of the start code, not the NAL.
    database.add<AnnexBToAvccTest>(
        "Several NALs, 4-byte separators",
        concat({START4, AUD, START4, SPS, START4, PPS, START4, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x07}, SPS,
            {0x00, 0x00, 0x00, 0x06}, PPS,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    database.add<AnnexBToAvccTest>(
        "Bytes before the first start code are skipped",
        concat({{0xab, 0xcd}, START3, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "No start code",
        Bytes{0x12, 0x34, 0x56},
        false, Bytes{}
    );
    database.add<AnnexBToAvccTest>(
        "Empty input",
        Bytes{},
        false, Bytes{}
    );
    database.add<AnnexBToAvccTest>(
        "Start code with no payload",
        START4,
        false, Bytes{}
    );

    //  avcC configuration record from Annex B extradata.
    database.add<AvccFromExtradataTest>(
        "SPS + PPS, 3-byte start codes",
        concat({START3, SPS, START3, PPS}),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "SPS + PPS, 4-byte start codes",
        concat({START4, SPS, START4, PPS}),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "Other NALs and repeated parameter sets are ignored",
        concat({
            START3, SEI,
            START3, SPS,
            START3, PPS,
            START3, Bytes{0x67, 0x4d, 0x00, 0x2a, 0x99},
            START3, Bytes{0x68, 0x01},
        }),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "SPS without PPS",
        concat({START3, SPS}),
        true, concat({
            {0x01, 0x64, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x07},
            SPS,
            {0x00},     //  No picture parameter sets.
        })
    );
    database.add<AvccFromExtradataTest>(
        "No SPS",
        concat({START3, PPS}),
        false, Bytes{}
    );
    database.add<AvccFromExtradataTest>(
        "SPS too short",
        concat({START3, Bytes{0x67, 0x64, 0x00}, START3, PPS}),
        false, Bytes{}
    );
    database.add<AvccFromExtradataTest>(
        "Empty input",
        Bytes{},
        false, Bytes{}
    );
}



}
