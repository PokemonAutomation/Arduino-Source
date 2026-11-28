/*  JoystickTools Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cstdint>
#include <cstring>
#include <format>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Controllers/Joystick.h"
#include "Controllers/JoystickTools.h"
#include "JoystickTools_Tests.h"

namespace PokemonAutomation{


namespace{


//  The refactored path, as the S1/S2 OEM controller callsites use it:
//  map the range, scale each axis to 12-bit, pack into the report's
//  3 stick bytes.
void encode_joystick(
    uint8_t data[3],
    double min_threshold, double max_threshold,
    const JoystickPosition& position
){
    JoystickPosition mapped = JoystickTools::map_to_range(
        min_threshold,
        max_threshold,
        position
    );
    uint16_t wx = JoystickTools::linear_float_to_u12(mapped.x);
    uint16_t wy = JoystickTools::linear_float_to_u12(mapped.y);
    data[0] = (uint8_t)wx;
    data[1] = (uint8_t)(wx >> 8 | wy << 4);
    data[2] = (uint8_t)(wy >> 4);
}


struct TestVector{
    JoystickPosition position;
    uint8_t procon[3];  //  thresholds 1874 / 320
    uint8_t joycon[3];  //  thresholds 1874 / 260
};

//  Both extremes and in-between values: the neutral position, the
//  clamped extremes (in- and out-of-range), the mid range, the
//  off-center rest region, and the exact rest-offset and saturation
//  points for each threshold set. The expected bytes are the
//  original encoder's output, checked before the refactor.
const TestVector k_test_vectors[] = {
    { { 0.0, 0.0 },               { 0x00, 0x08, 0x80 }, { 0x00, 0x08, 0x80 } },  //  neutral
    { { 1.0, 0.0 },               { 0xFF, 0x0F, 0x80 }, { 0xFF, 0x0F, 0x80 } },  //  positive x extreme
    { { 0.0, 1.0 },               { 0x00, 0xF8, 0xFF }, { 0x00, 0xF8, 0xFF } },  //  positive y extreme
    { { -1.0, -1.0 },             { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00 } },  //  negative diagonal extreme
    { { 2.0, -3.0 },              { 0xFF, 0x0F, 0x00 }, { 0xFF, 0x0F, 0x00 } },  //  out-of-range
    { { 0.0, 2.0 },               { 0x00, 0xF8, 0xFF }, { 0x00, 0xF8, 0xFF } },  //  out-of-range y
    { { 1.5, 0.5 },               { 0xFF, 0x0F, 0xC0 }, { 0xFF, 0x0F, 0xC0 } },  //  out-of-range, clamps x only
    { { 0.5, 0.0 },               { 0xB7, 0x0B, 0x80 }, { 0xD5, 0x0B, 0x80 } },  //  mid range
    { { 0.0, -0.5 },              { 0x00, 0x98, 0x44 }, { 0x00, 0xB8, 0x42 } },  //  mid range, negative y
    { { 0.3, -0.4 },              { 0x3A, 0x7A, 0x50 }, { 0x4C, 0xFA, 0x4E } },  //  both axes, mixed signs
    { { 0.70710678, 0.70710678 }, { 0xC5, 0x5C, 0xCC }, { 0xF0, 0x0C, 0xCF } },  //  45 degree diagonal
    { { 0.05, 0.05 },             { 0xC9, 0x98, 0x8C }, { 0xCC, 0xC8, 0x8C } },  //  off-center rest region
    { { 0.0849609375, 0.0 },      { 0x32, 0x09, 0x80 }, { 0x37, 0x09, 0x80 } },  //  the rest offset (1 - 1874 / 2048)
    { { 0.84375, 0.0 },           { 0xCC, 0x0D, 0x80 }, { 0xFF, 0x0D, 0x80 } },  //  the procon saturation point (1 - 320 / 2048)
    { { -0.84375, 0.0 },          { 0x33, 0x02, 0x80 }, { 0x00, 0x02, 0x80 } },  //  its negative
    { { 0.87255859375, 0.0 },     { 0xF9, 0x0D, 0x80 }, { 0x2E, 0x0E, 0x80 } },  //  the joycon saturation point (1 - 260 / 2048)
};

constexpr double PROCON_MIN_THRESHOLD = 1874. / 2048.;
constexpr double PROCON_MAX_THRESHOLD = 320. / 2048.;
constexpr double JOYCON_MIN_THRESHOLD = 1874. / 2048.;
constexpr double JOYCON_MAX_THRESHOLD = 260. / 2048.;



}



//  The refactor moved the stick's range mapping out of the S1/S2 OEM
//  controllers' encode_joystick() into JoystickTools::map_to_range()
//  (the thresholds as fractions of full scale), leaving the 12-bit
//  scaling and the 3-byte packing at the callsites. This checks the
//  new path against the original encoder's output for a set of
//  specific positions - both extremes (including out-of-range) and
//  in-between values - for the pro controller (1874 / 320) and the
//  joycon (1874 / 260) threshold sets.
class Test_JoystickTools_MapToRange : public UnitTest{
public:
    Test_JoystickTools_MapToRange()
        : UnitTest("JoystickTools::map_to_range - preserves the original encoder's numerics")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        for (size_t i = 0; i < sizeof(k_test_vectors) / sizeof(k_test_vectors[0]); i++){
            const TestVector& vector = k_test_vectors[i];

            uint8_t procon[3];
            encode_joystick(procon, PROCON_MIN_THRESHOLD, PROCON_MAX_THRESHOLD, vector.position);
            if (std::memcmp(procon, vector.procon, 3) != 0){
                return std::format(
                    "Pro controller thresholds: ({}, {}): expected {:02X} {:02X} {:02X}, got {:02X} {:02X} {:02X}",
                    vector.position.x, vector.position.y,
                    vector.procon[0], vector.procon[1], vector.procon[2],
                    procon[0], procon[1], procon[2]
                );
            }

            uint8_t joycon[3];
            encode_joystick(joycon, JOYCON_MIN_THRESHOLD, JOYCON_MAX_THRESHOLD, vector.position);
            if (std::memcmp(joycon, vector.joycon, 3) != 0){
                return std::format(
                    "Joycon thresholds: ({}, {}): expected {:02X} {:02X} {:02X}, got {:02X} {:02X} {:02X}",
                    vector.position.x, vector.position.y,
                    vector.joycon[0], vector.joycon[1], vector.joycon[2],
                    joycon[0], joycon[1], joycon[2]
                );
            }
        }
        return true;
    }
};



void add_tests_JoystickTools(UnitTestDatabase& database){
    database.add<Test_JoystickTools_MapToRange>();
}



}
