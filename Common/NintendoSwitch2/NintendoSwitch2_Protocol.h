/*  Nintendo Switch 2 Controller Protocol
 * 
 *  From: https://github.com/PokemonAutomation/
 *
 *  The unified command protocol spoken by the Switch 2 OEM controllers
 *  (Pro Controller 2, Joy-Con 2 L/R, NSO GameCube Controller) on both
 *  of their transports: the USB vendor-bulk endpoints (0x02 OUT /
 *  0x82 IN, transport byte 0x00) and the proprietary BLE GATT
 *  (transport byte 0x01). One protocol, two transports: every command
 *  is an 8-byte header plus a payload.
 *
 *  The USB console-side handshake observed on a genuine console (the
 *  order is not strict; implement an order-independent dispatcher
 *  keyed on command+subcommand and only stream input once a report is
 *  selected and the features enabled):
 *
 *    03/0D  Initialise USB (payload: 01 00 + host BT address, 6 B)
 *    07/01  First-init (no payload; response: 1 flag byte)
 *    16/01  (no payload; response: 24 zero bytes)
 *    15/04  Pairing: exchange keys (A1 host key; response: B1)
 *    15/01  Pairing: exchange addresses (host list; response: controller address)
 *    15/02  Pairing: confirm LTK (A2 challenge; response: B2 = AES-128-ECB(LTK, A2))
 *    15/03  Pairing: finalise
 *    09/07  Player LEDs (player bitmask + 7 B zero)
 *    0C/02  Set feature mask, then 0C/04 enable
 *    02/04  Memory reads (factory / calibration blocks)
 *    03/0A  Select input report (0x05 combined or the native report)
 *    10/01  Firmware version info
 *
 *  The 0x15 pairing exchange can also be replaced by the 0x03/07
 *  "send pairing info" shortcut (host address + LTK) followed by
 *  0x03/09 store - how "pairing by USB with link-key transfer to
 *  Bluetooth" works.
 *
 *  Sources (byte-exact): ndeadly's switch2_controller_research
 *  (commands.md, hid_reports.md), PicoSwitch2's Switch 2 USB spec
 *  (docs/switch2/usb-spec.md §4-§5, firmware-versioning.md) and
 *  linux-switch2's PROTOCOL.md.
 *
 *  Shared module: the firmware target implements the device side
 *  (parsing commands, building responses) and the computer side
 *  implements the host/console side. No board or stack dependencies;
 *  byte-identical between the Firmware and Arduino-Source repos.
 */

#ifndef PokemonAutomation_NintendoSwitch2_Protocol_H
#define PokemonAutomation_NintendoSwitch2_Protocol_H

#include <cstdint>

namespace PokemonAutomation{
namespace NintendoSwitch2{
namespace Protocol{

//  Header byte 2: the transport this command travels on.
constexpr uint8_t TRANSPORT_USB = 0x00;
constexpr uint8_t TRANSPORT_BLE = 0x01;

//  Header byte 1: the direction of the command.
constexpr uint8_t DIRECTION_HOST_TO_DEVICE = 0x91;
constexpr uint8_t DIRECTION_DEVICE_TO_HOST = 0x01;

//  The command IDs (header byte 0).
namespace Command{
constexpr uint8_t NFC = 0x01;               //  amiibo / NFC
constexpr uint8_t FLASH = 0x02;             //  flash memory
constexpr uint8_t INITIALISE = 0x03;
constexpr uint8_t UNKNOWN_0x04 = 0x04;
constexpr uint8_t UNKNOWN_0x05 = 0x05;
constexpr uint8_t UNKNOWN_0x06 = 0x06;
constexpr uint8_t UNKNOWN_0x07 = 0x07;      //  first command of every init sequence
constexpr uint8_t CHARGING_GRIP = 0x08;
constexpr uint8_t PLAYER_LEDS = 0x09;
constexpr uint8_t VIBRATION = 0x0A;
constexpr uint8_t BATTERY = 0x0B;
constexpr uint8_t FEATURE_SELECT = 0x0C;
constexpr uint8_t FIRMWARE_UPDATE = 0x0D;   //  bricking risk - do not implement early
constexpr uint8_t UNKNOWN_0x0E = 0x0E;
constexpr uint8_t UNKNOWN_0x0F = 0x0F;
constexpr uint8_t FIRMWARE_INFO = 0x10;
constexpr uint8_t UNKNOWN_0x11 = 0x11;
constexpr uint8_t UNKNOWN_0x12 = 0x12;
constexpr uint8_t UNKNOWN_0x13 = 0x13;
constexpr uint8_t UNKNOWN_0x14 = 0x14;
constexpr uint8_t PAIRING = 0x15;
constexpr uint8_t UNKNOWN_0x16 = 0x16;      //  24 zero bytes of response
constexpr uint8_t UNKNOWN_0x17 = 0x17;
constexpr uint8_t UNKNOWN_0x18 = 0x18;
}

//  0x02 - flash memory subcommands.
namespace Flash{
constexpr uint8_t READ_BLOCK = 0x01;        //  fixed 0x40 B block
constexpr uint8_t WRITE_BLOCK = 0x02;       //  fixed 0x40 B block
constexpr uint8_t ERASE_SECTOR = 0x03;      //  0x1000 B sector -> 0xFF
constexpr uint8_t READ = 0x04;              //  up to 0x50 B (USB) / 0x4F B (BLE)
constexpr uint8_t WRITE = 0x05;             //  up to 0x80 B, only >= 0x1F5000
}

//  0x03 - initialisation subcommands.
namespace Initialise{
constexpr uint8_t BLUETOOTH_WAKE = 0x01;
constexpr uint8_t BLUETOOTH_CANCEL = 0x02;
constexpr uint8_t ENABLE_USB_HID_REPORTS = 0x03;
constexpr uint8_t SEND_PAIRING_INFO = 0x07; //  host address + LTK (22 B payload)
constexpr uint8_t CLEAR_PAIRING_INFO = 0x08;
constexpr uint8_t STORE_PAIRING_INFO = 0x09;
constexpr uint8_t SELECT_INPUT_REPORT = 0x0A;
constexpr uint8_t INITIALISE_USB = 0x0D;    //  host address; required before USB input
constexpr uint8_t UNKNOWN_0x0C = 0x0C;
}

//  0x0C - feature select subcommands.
namespace Feature{
constexpr uint8_t GET_INFO = 0x01;
constexpr uint8_t SET_MASK = 0x02;
constexpr uint8_t CLEAR_MASK = 0x03;
constexpr uint8_t ENABLE = 0x04;
constexpr uint8_t DISABLE = 0x05;
constexpr uint8_t CONFIGURE = 0x06;

//  Feature mask bits (the 0x0C payload byte 0).
constexpr uint8_t MASK_BUTTONS = 0x01;
constexpr uint8_t MASK_STICKS = 0x02;
constexpr uint8_t MASK_IMU = 0x04;
constexpr uint8_t MASK_OPTICAL = 0x10;     //  Joy-Con 2 only
constexpr uint8_t MASK_RUMBLE = 0x20;
constexpr uint8_t MASK_MAGNETOMETER = 0x80;
}

//  0x15 - Bluetooth pairing subcommands.
namespace Pairing{
constexpr uint8_t EXCHANGE_ADDRESSES = 0x01;
constexpr uint8_t CONFIRM_LTK = 0x02;
constexpr uint8_t FINALISE = 0x03;
constexpr uint8_t EXCHANGE_KEYS = 0x04;
}

//  The 8-byte command header.
//
//  Requests carry the payload length in payload_len; responses echo
//  the command and subcommand and carry the transport-specific ack
//  tail instead: USB 00 F8 00 00, BLE 10 78 00 00.
#pragma pack(push, 1)
struct S2_CommandHeader{
    uint8_t command;
    uint8_t direction;
    uint8_t transport;
    uint8_t subcommand;
    uint8_t unknown;      //  0x00 on requests; 0x00 (USB) / 0x10 (BLE) on responses
    uint8_t payload_len;  //  requests: payload byte count; responses: USB 0xF8 / BLE 0x78
    uint8_t reserved[2];  //  00 00
};
#pragma pack(pop)
static_assert(sizeof(S2_CommandHeader) == 8, "S2 command header is 8 bytes");

//  Build a host-to-device request header.
S2_CommandHeader S2_MakeRequestHeader(
    uint8_t command, uint8_t subcommand, uint8_t transport, uint8_t payload_len
);

//  Build a device-to-host response header echoing the command/subcommand.
S2_CommandHeader S2_MakeResponseHeader(uint8_t command, uint8_t subcommand, uint8_t transport);

//  The USB / BLE response ack tails.
constexpr uint8_t S2_USB_RESPONSE_ACK[4] = {0x00, 0xF8, 0x00, 0x00};
constexpr uint8_t S2_BLE_RESPONSE_ACK[4] = {0x10, 0x78, 0x00, 0x00};

//  ---------------------------------------------------------------------
//  0x15 - Bluetooth pairing
//
//  The controller's key B1 is a fixed public constant shared by the
//  S2 controller family (byte-reversed on the wire, like all 16-byte
//  pairing values). LTK = reverse(A1) XOR reverse(B1); the 0x15/02
//  challenge response is B2 = AES-128-ECB(LTK, reverse(A2)).
//
//  0x15/01 request payload:  00 <count> <6 B address x count>
//  0x15/01 response payload:  01 04 01 <6 B controller address>
//  0x15/02 request payload:   00 <16 B A2>
//  0x15/02 response payload:  01 <16 B B2>
//  0x15/03 request payload:   00
//  0x15/03 response payload:  01
//  0x15/04 request payload:   00 <16 B A1>
//  0x15/04 response payload:  01 <16 B B1>
//
//  0x03/07 "send pairing info" request payload (the USB pairing
//  shortcut): <6 B host address> <16 B LTK>. 0x03/08 clears it,
//  0x03/09 stores it (into the bond table at 0x1FA000).
//  ---------------------------------------------------------------------

//  B1 in wire byte-order (the reverse of the public constant).
constexpr uint8_t S2_DEVICE_KEY_B1[16] = {
    0x5C, 0xF6, 0xEE, 0x79, 0x2C, 0xDF, 0x05, 0xE1,
    0xBA, 0x2B, 0x63, 0x25, 0xC4, 0x1A, 0x5F, 0x10,
};

//  Single-block AES-128-ECB encrypt (the pairing crypto needs only one
//  block; no padding, no modes).
void S2_Aes128EncryptBlock(const uint8_t key[16], const uint8_t in[16], uint8_t out[16]);

//  LTK = reverse(a1_wire) XOR reverse(S2_DEVICE_KEY_B1).
void S2_DeriveLtk(const uint8_t a1_wire[16], uint8_t ltk[16]);

//  b2_wire = AES-128-ECB(ltk, reverse(a2_wire)).
void S2_PairingChallenge(const uint8_t ltk[16], const uint8_t a2_wire[16], uint8_t b2_wire[16]);

//  The 0x03/07 "send pairing info" payload (22 B).
#pragma pack(push, 1)
struct S2_SendPairingInfo{
    uint8_t host_address[6];
    uint8_t ltk[16];
};
#pragma pack(pop)
static_assert(sizeof(S2_SendPairingInfo) == 22, "0x03/07 payload is 22 bytes");

//  ---------------------------------------------------------------------
//  0x02 - flash memory payloads
//
//  0x02/04 read request:      <len> 7E 00 00 <address LE x 4>
//  0x02/04 read response:     <len> 00 00 00 <address LE x 4> <data>
//  0x02/05 write request:     <len> 7E 00 00 <address LE x 4> <data>
//  0x02/05 write response:    <status> 00 00 00 <address LE x 4>   (status 0x00 OK / 0x81 not writable)
//  0x02/01/02/03 payloads are <address LE x 4> plus, for 0x01/02, a
//  fixed 0x40 B block.
//  ---------------------------------------------------------------------

//  0x02/04 request payload (8 B).
#pragma pack(push, 1)
struct S2_FlashReadRequest{
    uint8_t len;          //  <= 0x50 (USB) / 0x4F (BLE)
    uint8_t unknown_0x7E; //  always 0x7E on requests
    uint8_t padding[2];
    uint8_t address[4];   //  little-endian
};
//  0x02/04 response payload head (8 B, data follows).
struct S2_FlashReadResponseHead{
    uint8_t len;
    uint8_t padding[3];
    uint8_t address[4];   //  little-endian
};
//  0x02/05 request payload head (8 B, data follows).
struct S2_FlashWriteRequestHead{
    uint8_t len;          //  <= 0x80
    uint8_t unknown_0x7E;
    uint8_t padding[2];
    uint8_t address[4];   //  little-endian, must be >= 0x1F5000
};
//  0x02/05 response payload (8 B).
struct S2_FlashWriteResponse{
    uint8_t status;       //  0x00 OK, 0x81 address not writable
    uint8_t padding[3];
    uint8_t address[4];   //  little-endian
};
#pragma pack(pop)
static_assert(sizeof(S2_FlashReadRequest) == 8, "0x02/04 request payload is 8 bytes");
static_assert(sizeof(S2_FlashReadResponseHead) == 8, "0x02/04 response head is 8 bytes");
static_assert(sizeof(S2_FlashWriteRequestHead) == 8, "0x02/05 request head is 8 bytes");
static_assert(sizeof(S2_FlashWriteResponse) == 8, "0x02/05 response is 8 bytes");

//  ---------------------------------------------------------------------
//  Firmware / identity info
//
//  0x10/01 response payload (12 B):
//    <controller fw major.minor.micro> <type> <bt patch major.minor.micro>
//    <pad> <dsp fw major.minor.micro> <pad>
//  type: 00 Joy-Con L, 01 Joy-Con R, 02 Pro, 03 GameCube. Version bytes
//  are the decimal values (0x04 = 4); DSP reads FF FF FF when absent.
//
//  The console also queries two blocks over EP0 vendor control before
//  it touches the bulk command channel:
//    bRequest 0x03: the 64 B identity block (the first 64 B of factory
//                   flash at 0x13000)
//    bRequest 0x02: the 16 B firmware/version block
//  ---------------------------------------------------------------------

constexpr uint8_t S2_CONTROLLER_TYPE_JOYCON_L = 0x00;
constexpr uint8_t S2_CONTROLLER_TYPE_JOYCON_R = 0x01;
constexpr uint8_t S2_CONTROLLER_TYPE_PRO = 0x02;
constexpr uint8_t S2_CONTROLLER_TYPE_GAMECUBE = 0x03;

#pragma pack(push, 1)
//  0x10/01 response payload (12 B).
struct S2_FirmwareInfo{
    uint8_t controller_firmware[3];
    uint8_t controller_type;
    uint8_t bt_patch[3];
    uint8_t padding;
    uint8_t dsp_firmware[3];
    uint8_t padding2;
};
//  EP0 vendor control bRequest 0x02 payload (16 B).
struct S2_Ep0FirmwareInfo{
    uint8_t controller_firmware[3];
    uint8_t reserved[3];
    uint8_t bt_patch_major;
    uint8_t reserved2[3];
    uint8_t bd_addr[6];
};
#pragma pack(pop)
static_assert(sizeof(S2_FirmwareInfo) == 12, "0x10/01 payload is 12 bytes");
static_assert(sizeof(S2_Ep0FirmwareInfo) == 16, "EP0 firmware block is 16 bytes");

constexpr uint16_t S2_EP0_IDENTITY_SIZE = 64;  //  bRequest 0x03

}}}

#endif
