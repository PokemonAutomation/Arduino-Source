/*  Nintendo Switch 2 - OEM Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_OemController_State_H
#define PokemonAutomation_NintendoSwitch2_OemController_State_H

#include <stdbool.h>
#include <stdint.h>

//  The report payloads place 16-bit fields on odd byte offsets, so the
//  report structs are packed to keep a byte-exact wire layout.
#pragma pack(push, 1)

namespace PokemonAutomation{
namespace NintendoSwitch2{



//
//  Notes
//
//  Every S2 controller has two input report formats:
//
//  - 0x05 (common): one shape for every controller type. This is the
//    "grip" format - what a charging grip presents for a docked Joy-Con 2
//    L+R pair, and the stable alternate used by PC tools. All controllers
//    can produce it.
//  - Native side reports 0x07/0x08/0x09/0x0A (Joy-Con 2 L, Joy-Con 2 R,
//    Pro Controller 2, NSO GameCube): the power-on default for a
//    standalone controller of that type, and what the console uses for it.
//
//  The host picks the format with command 0x03 sub 0x0A over USB (the
//  controller streams the selected report; power-up default is the native
//  side report for that controller type). Over BLE both formats exist as
//  separate GATT characteristics (0x05 at handle 0x000a, native at
//  0x000e) and the host subscribes to whichever it wants via the CCCs;
//  the 0x03 sub 0x0A value in the known-good BLE startup sequence is
//  0x09 even for Joy-Con 2, so it is not a literal report ID there.
//
//  All structs below are the report payload MINUS the fields the
//  firmware fills in - the same "header" information the S1 states omit:
//
//  - 0x05: 4-byte report counter at the start (struct is 59 bytes,
//    63 on the wire)
//  - Native reports: 1-byte counter + 1-byte power info at the start
//    ([0] ext power, [1] charging, [2:5] battery level, [6:7] reserved)
//    (struct is 61 bytes, 63 on the wire)
//
//  The USB report ID byte is also added by the firmware (BLE
//  notifications omit it; USB reports prepend it). The unknown_0xNN
//  field names below keep their on-wire offsets.
//
//  Multi-byte fields are little-endian on the wire. Sticks are packed
//  12-bit: X = b0 | ((b1 & 0x0f) << 8); Y = (b1 >> 4) | (b2 << 4);
//  neutral is ~0x800 but the true center comes from the controller's
//  flash calibration.
//

//
//  Console <-> Controller
//


//  16-byte LRA frame per motor (HD rumble). The Pro Controller 2 drives
//  both frames; a Joy-Con 2 drives the single frame for its side; the
//  NSO GameCube controller uses a separate 4-byte motor format
//  (OemController_RumbleStateGC). The internal packing (two
//  frequency/amplitude pairs) is partially opaque; zero amplitude is
//  silent.
struct OemController_RumbleState{
    uint8_t left[16];
    uint8_t right[16];
};

extern const OemController_RumbleState OemController_RumbleState_NEUTRAL_STATE;


//  NSO GameCube controller motor (4-byte packed frame, output report 0x03).
struct OemController_RumbleStateGC{
    uint8_t motor[4];
};

extern const OemController_RumbleStateGC OemController_RumbleStateGC_NEUTRAL_STATE;


//
//  Report 0x05 (common input report - all controller types)
//


struct OemController_State0x05_Optical{
    uint16_t position_x;   // absolute position (Joy-Con 2 only; zeros elsewhere)
    uint16_t position_y;
    uint16_t status0;      // low byte tracks the native coveredness/lift-off byte
    uint16_t status1;      // lift-off distance?; semantics open
};

struct OemController_State0x05_Magnetometer{
    int16_t x;   // 0.15 uT/LSB (feature bit 7)
    int16_t y;
    int16_t z;
};

struct OemController_State0x05_Motion{
    uint32_t timestamp;   // microseconds (some controllers report it incorrectly)
    int16_t  temperature; // deviation from 25C (1/126.9 C/LSB)
    int16_t  accel_x;     // default FSR +/-8G (8.0/32767 G/LSB)
    int16_t  accel_y;
    int16_t  accel_z;
    int16_t  gyro_x;      // default FSR +/-2000dps (2000.0/32767 dps/LSB)
    int16_t  gyro_y;
    int16_t  gyro_z;
};

//  The fast-path input fields of report 0x05: an exact prefix of
//  OemController_State0x05 (buttons, the unknown pair, sticks), so the
//  firmware can memcpy it over the head of the state. The GameCube
//  analog triggers sit after the motion block, so no contiguous prefix
//  can reach them - the GC target uses report 0x0A, where they are part
//  of the 0x0A buttons prefix.
struct OemController_State0x05_Buttons{
    uint8_t buttons[4];
    uint8_t unknown_0x08[2];
    uint8_t left_stick[3];
    uint8_t right_stick[3];
};

struct OemController_State0x05{
    //  4 button bytes (MSB -> LSB):
    //  byte0: ZR R SL-R SR-R A B X Y
    //  byte1: C Capture Home Lstickclick Rstickclick Plus Minus
    //  byte2: ZL L SL-L SR-L Left Right Up Down
    //  byte3: - - - Headset - - GL GR
    uint8_t buttons[4];

    uint8_t  unknown_0x08[2];
    uint8_t  left_stick[3];
    uint8_t  right_stick[3];

    OemController_State0x05_Optical optical;

    uint8_t unknown_0x18;

    OemController_State0x05_Magnetometer magnetometer;

    uint16_t battery_voltage_mv;
    uint8_t  charging_state;    // rises and settles on 0x34 when charging, 0x20 when full
    uint16_t battery_current;   // encoding unknown (feature bit 5)
    uint8_t  unknown_0x24[5];
    uint8_t  unknown_0x29;      // always 0x01

    OemController_State0x05_Motion motion;

    uint8_t left_trigger;     // NSO GameCube only
    uint8_t right_trigger;    // NSO GameCube only
    uint8_t reserved;
};

extern const OemController_State0x05 OemController_State0x05_NEUTRAL_STATE;

bool OemController_State0x05_equals(
    const OemController_State0x05* state0,
    const OemController_State0x05* state1
);


//
//  Report 0x07 (Joy-Con 2 Left native)
//


struct OemController_State0x07_Optical{
    int16_t delta_x;
    int16_t delta_y;
    uint8_t coveredness;   // ~0xff uncovered/lifted, ~0x00 resting on a surface
};

//  Exact prefix of OemController_State0x07.
struct OemController_State0x07_Buttons{
    uint8_t buttons[2];
    uint8_t unknown_0x04;
    uint8_t stick[3];
};

struct OemController_State0x07{
    uint8_t buttons0;      // MSB->LSB: Lstickclick Minus ZL L | Up Left Right Down
    uint8_t buttons1;      // MSB->LSB: SL SR | - - - - - | Capture
    uint8_t unknown_0x04;  // observed 0x07
    uint8_t stick[3];
    uint8_t unknown_0x08;
    OemController_State0x07_Optical optical;
    uint8_t motion_length; // 0x28 ordinary / 0x1e compact / 0x00
    uint8_t motion_data[40];
    uint8_t reserved[8];
};

extern const OemController_State0x07 OemController_State0x07_NEUTRAL_STATE;

bool OemController_State0x07_equals(
    const OemController_State0x07* state0,
    const OemController_State0x07* state1
);


//
//  Report 0x08 (Joy-Con 2 Right native)
//
//  Same as 0x07 with an extra NFC state byte (the Right Joy-Con carries
//  the NFC reader) between the optical block and the motion length.
//


//  Exact prefix of OemController_State0x08.
struct OemController_State0x08_Buttons{
    uint8_t buttons[2];
    uint8_t unknown_0x04;
    uint8_t stick[3];
};

struct OemController_State0x08{
    uint8_t buttons0;      // MSB->LSB: Rstickclick Plus ZR R | X Y A B
    uint8_t buttons1;      // MSB->LSB: SL SR | C | - - - | Home
    uint8_t unknown_0x04;  // observed 0x07
    uint8_t stick[3];
    uint8_t unknown_0x08;
    OemController_State0x07_Optical optical;
    uint8_t nfc_state;     // 0x00-0x07, 0x00 = idle
    uint8_t motion_length;
    uint8_t motion_data[40];
    uint8_t reserved[7];
};

extern const OemController_State0x08 OemController_State0x08_NEUTRAL_STATE;

bool OemController_State0x08_equals(
    const OemController_State0x08* state0,
    const OemController_State0x08* state1
);


//
//  Report 0x09 (Pro Controller 2 native)
//


//  Exact prefix of OemController_State0x09.
struct OemController_State0x09_Buttons{
    uint8_t buttons[3];
    uint8_t left_stick[3];
    uint8_t right_stick[3];
};

struct OemController_State0x09{
    uint8_t buttons[3];
    //  MSB->LSB, byte0: Rstickclick Plus ZR R | X Y A B
    //  MSB->LSB, byte1: Lstickclick Minus ZL L | Up Left Right Down
    //  MSB->LSB, byte2: - - - C GL GR Capture Home
    uint8_t left_stick[3];
    uint8_t right_stick[3];
    uint8_t unknown_0x0B;  // 0x38 if feature bit 5 is set, otherwise 0x30
    uint8_t nfc_state;     // 0x00-0x07, 0x00 = idle
    uint8_t headset_audio_state;   // 0x07/0x0F headset, 0x05/0x0D headphones, 0x00 otherwise
    uint8_t motion_length; // 0x28 ordinary / 0x1e compact / 0x00
    uint8_t motion_data[40];
    uint8_t reserved[8];
};

extern const OemController_State0x09 OemController_State0x09_NEUTRAL_STATE;

bool OemController_State0x09_equals(
    const OemController_State0x09* state0,
    const OemController_State0x09* state1
);


//
//  Report 0x0A (NSO GameCube native)
//


//  Exact prefix of OemController_State0x0A.
struct OemController_State0x0A_Buttons{
    uint8_t buttons[3];
    uint8_t left_stick[3];
    uint8_t right_stick[3];
    uint8_t unknown_0x0B;
    uint8_t left_trigger;
    uint8_t right_trigger;
};

struct OemController_State0x0A{
    uint8_t buttons[3];
    //  MSB->LSB, byte0: Rstickclick Plus R Z | X Y A B
    //  MSB->LSB, byte1: Lstickclick Minus L ZL | Up Left Right Down
    //  MSB->LSB, byte2: - - - C | - - | Capture Home
    uint8_t left_stick[3];
    uint8_t right_stick[3];
    uint8_t unknown_0x0B;  // 0x38 if feature bit 5 is set, otherwise 0x30
    uint8_t left_trigger;
    uint8_t right_trigger;
    uint8_t motion_length;
    uint8_t motion_data[40];
    uint8_t reserved[8];
};

extern const OemController_State0x0A OemController_State0x0A_NEUTRAL_STATE;

bool OemController_State0x0A_equals(
    const OemController_State0x0A* state0,
    const OemController_State0x0A* state1
);


//
//  Motion payload (inside the native reports)
//
//  motion_data[] holds the raw payload selected by motion_length:
//
//  - 0x28 (40 B, "ordinary"): bit-packed scheduler time, quaternion
//    (smallest-three, 21-bit components, 2-bit omitted-component
//    selector) plus explicit accelerometer (three samples, mixed
//    14/13-bit) and gyroscope (two samples, 15-bit) values on a ~48.75 ms
//    cadence with non-uniform internal timing.
//  - 0x1e (30 B, "compact"): quaternion (25-bit components) plus 16-bit
//    acceleration on a ~250 Hz cadence over USB.
//
//  The two formats use DIFFERENT quaternion codecs (the BLE and USB
//  compact forms also differ from each other), and several interleaved
//  bits are semantically unresolved, so the payload is kept raw here.
//  Field-level decoding is a CC-side concern; see the S2 protocol
//  analysis (Docs/Switch2/S2-Protocol-Analysis.md) and the linux-switch2
//  PROTOCOL.md sections 20-27 for the bit maps.
//



//
//  Controller <-> CC
//


struct OemController_Status{
    //  Bit 0: Connected
    //  Bit 1: Ready
    //  Bit 2: Paired
    uint8_t status;

    uint8_t player_lights;

    //  The controller's BLE address (the S2 stores the host's address,
    //  so this is the base's own address for reconnection purposes).
    uint8_t paired_mac_address[6];
};

enum class DisconnectReason{
    HOST_SHUTDOWN,
    CONNECTION_REJECTED,
};



}
}

#pragma pack(pop)

#endif
