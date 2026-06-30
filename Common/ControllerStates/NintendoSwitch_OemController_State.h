/*  Nintendo Switch - OEM Controller State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_OemController_State_H
#define PokemonAutomation_NintendoSwitch_OemController_State_H

#include <stdbool.h>
#include <stdint.h>
#include <cstdint>
#include "string.h"

//#define PACKED __attribute__((packed))

namespace PokemonAutomation{
namespace NintendoSwitch{



//
//  Console <-> Controller
//


struct OemController_RumbleState{
    uint8_t left[4];
    uint8_t right[4];
};

extern const OemController_RumbleState OemController_RumbleState_NEUTRAL_STATE;


//
//  Report 0x3f (no longer used)
//

struct OemController_State0x3f{
    uint8_t button0;
    uint8_t button1;
    uint8_t dpad;
    uint8_t left_joystick[4];
    uint8_t right_joystick[4];
};

extern const OemController_State0x3f OemController_State0x3f_NEUTRAL_STATE;


//
//  Report 0x30
//

struct OemController_State0x30_Buttons{
    uint8_t button3;
    uint8_t button4;
    uint8_t button5;
    uint8_t left_joystick[3];
    uint8_t right_joystick[3];
    uint8_t vibrator;
};

static inline void write_bits(uint8_t* raw, int bit, int len, uint32_t value) {
    uint64_t mask = ((1ULL << len) - 1);
    value &= (uint32_t)mask;
    int byte = bit / 8;
    int shift = bit % 8;
    // Touch only the bytes covered by this field, including at the packet end.
    const int byte_count = (shift + len + 7) / 8;
    uint64_t existing = 0;
    for (int i = 0; i < byte_count; i++)
        existing |= (uint64_t)raw[byte + i] << (i * 8);
    existing &= ~(mask << shift);
    existing |= ((uint64_t)value << shift);
    for (int i = 0; i < byte_count; i++)
        raw[byte + i] = (uint8_t)(existing >> (i * 8));
}


#pragma pack(push, 1)

struct OemController_State0x30_Gyro{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rotation_x;
    int16_t rotation_y;
    int16_t rotation_z;
};

struct OemController_State0x30_GyroQuaternion{
    uint8_t raw[36];

    void set_quat_accel_x0(int16_t v) { memcpy(&raw[0], &v, 2); }
    void set_quat_accel_y0(int16_t v) { memcpy(&raw[2], &v, 2); }
    void set_quat_accel_z0(int16_t v) { memcpy(&raw[4], &v, 2); }
    void set_quat_accel_x1(int16_t v) { memcpy(&raw[12], &v, 2); }
    void set_quat_accel_y1(int16_t v) { memcpy(&raw[14], &v, 2); }
    void set_quat_accel_z1(int16_t v) { memcpy(&raw[16], &v, 2); }
    void set_quat_accel_x2(int16_t v) { memcpy(&raw[24], &v, 2); }
    void set_quat_accel_y2(int16_t v) { memcpy(&raw[26], &v, 2); }
    void set_quat_accel_z2(int16_t v) { memcpy(&raw[28], &v, 2); }

    void set_packing_mode(uint32_t v) { write_bits(raw, 48, 2, v); }
    void set_max_index(uint32_t v) { write_bits(raw, 50, 2, v); }
    void set_last_sample_0(uint32_t v) { write_bits(raw, 52, 21, v); }
    void set_last_sample_1(uint32_t v) {
        write_bits(raw, 73, 7, v & 0x7F);
        write_bits(raw, 80, 14, v >> 7);
    }
    void set_last_sample_2(uint32_t v) {
        write_bits(raw, 94, 2, v & 0x3);
        write_bits(raw, 144, 19, v >> 2);
    }
    void set_delta_last_first_0(uint32_t v) { write_bits(raw, 163, 13, v); }
    void set_delta_last_first_1(uint32_t v) { write_bits(raw, 176, 13, v); }
    void set_delta_last_first_2(uint32_t v) {
        write_bits(raw, 189, 3, v & 0x7);
        write_bits(raw, 240, 10, v >> 3);
    }
    void set_delta_mid_avg_0(uint32_t v) { write_bits(raw, 250, 7, v); }
    void set_delta_mid_avg_1(uint32_t v) { write_bits(raw, 257, 7, v); }
    void set_delta_mid_avg_2(uint32_t v) { write_bits(raw, 264, 7, v); }
    void set_timestamp_start(uint32_t v) {
        write_bits(raw, 271, 1, v & 0x1);
        write_bits(raw, 272, 10, v >> 1);
    }
    void set_timestamp_count(uint32_t v) { write_bits(raw, 282, 6, v); }

};

#pragma pack(pop)

static_assert(sizeof(OemController_State0x30_Gyro) == 12, "Gyro struct must be 12 bytes");
static_assert(sizeof(OemController_State0x30_GyroQuaternion) == 36, "Quaternion gyro struct must be 36 bytes");

struct OemController_State0x30_GyroX3{
    struct Samples{
        OemController_State0x30_Gyro time0;
        OemController_State0x30_Gyro time1;
        OemController_State0x30_Gyro time2;
    };

    union{
        Samples samples;
        OemController_State0x30_GyroQuaternion quaternion;
    };
};

static_assert(sizeof(OemController_State0x30_GyroX3) == 36, "GyroX3 struct must be 36 bytes");

struct OemController_State0x30{
    OemController_State0x30_Buttons buttons;
    OemController_State0x30_GyroX3 gyro;
};

static_assert(sizeof(OemController_State0x30) == 46, "Report 0x30 state must be 46 bytes");

extern const OemController_State0x30 OemController_State0x30_NEUTRAL_STATE;

bool OemController_State0x30_equals(
    const OemController_State0x30* state0,
    const OemController_State0x30* state1
);


//
//  Controller <-> CC
//

struct OemController_Status{
    //  Bit 0: Connected
    //  Bit 1: Ready
    //  Bit 2: Paired
    uint8_t status;

    uint8_t player_lights;

    uint8_t paired_mac_address[6];
    
    // 0 = Standard
    // 1 = Quaternion
    //uint8_t gyro_protocol = 1;
};

enum class DisconnectReason{
    HOST_SHUTDOWN,
    CONNECTION_REJECTED,
};



}
}
#endif
