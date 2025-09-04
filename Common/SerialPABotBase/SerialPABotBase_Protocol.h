/*  Serial PABotBase Message Protocol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Pokemon Automation Bot-Base implements reliable data transmissions over
 *  serial communication. This is done by checksumming messages along with a
 *  protocol that is tolerant to data drops.
 *
 *  This file describes the message protocol. The data being transmitted is raw
 *  binary and not is readable text.
 *
 *
 *  Message Format:
 *      byte 0: Length of the entire message. (bits are inverted)
 *      byte 1: Message Type
 *      byte X: Optional data of variable length.
 *      Last 4 bytes: CRC32C of the entire message except these last 4 bytes.
 *
 *      Thus there are 6 bytes of overhead for each message.
 *
 *
 *  There are currently 4 categories of message types:
 *
 *      1.  Info: These are simple one-way messages. They do not need to be
 *          acked and may be dropped without adversely affecting anything.
 *
 *      2.  Ack/Response: These are messages sent in response to an earlier
 *          message that was received.
 *
 *      3.  Request: The sender requests the receiver to do something simple.
 *          The receiver must respond with an ack.
 *
 *          This is used for things that take no clock time. For example, querying
 *          for program identifiers, turning on/off LEDs, or setting flags in the
 *          program to change its behavior in the future.
 *
 *      4.  Command: The sender requests the receiver to do a large asynchronous
 *          operation. The receiver must ack this message. Once the command is
 *          finished, the receiver must send a request message back to the sender
 *          to indicate that the command is finished.
 *
 *          This is used for issuing button presses or other subroutines that
 *          consume time.
 *
 *
 *  General Protocol:
 *
 *      1.  Every time you send a new request/command message, you increment
 *          your sequence number (seqnum) by 1.
 *
 *      2.  If you receive an invalid message (bad length or bad checksum), ignore
 *          the first byte and attempt to parse the next byte as the start of a
 *          new message.
 *
 *      3.  If you receive a zero for the 1st byte of a message, ignore it and
 *          attempt to parse the next byte as the start of a new message.
 *
 *      4.  At any point, you can send a bunch of zero bytes. This will cause
 *          the receiver to re-synchronize.
 *
 *      5.  If you receive a request/command message, you must send the appropriate
 *          ack/response message using the same seqnum.
 *
 *      6.  If you receive a command message, you must first ack the message itself.
 *          Once the command is finished, you must send a request referencing the
 *          command to indicate that it is finished. You will receive an ack for
 *          this short (finishing) command, and if you don't, send it again until
 *          you do. If the command finishes immediately, you can skip the ack and
 *          just send the finish request.
 *
 *      7.  If you send a request/command message and don't get a response after
 *          a time limit, you should resend the message with the same seqnum.
 *
 *      8.  If you receive a request/command that has a seqnum ahead of what you
 *          are expecting, it means an earlier request/command was dropped.
 *          Do not process the request/command since you will lose ordering.
 *
 *      9.  If you receive a request/command that has an old seqnum, it is a
 *          retransmit. Send an ack for it, but don't process it again. (idempotency)
 *
 *
 *  Failure Analysis:
 *
 *      -   Corrupted messages will either fail checksum or will have an invalid
 *          length/type. These are simply ignored and dropped.
 *
 *      -   If either sender or receiver gets out-of-sync and loses track of
 *          message boundaries, it will eventually find the boundary again by
 *          simply trying to parse every byte as the start of a new message and
 *          verifying the length and CRC.
 *
 *      -   If a request/command is dropped, no ack will be received. The sender
 *          will eventually send the command again. (#7)
 *
 *      -   If an ack is dropped, the sender will eventually resend the
 *          request/command again. The receiver will see the duplicate
 *          request/command and ack it. But the receiver will not process it
 *          again to preserve idempotency. (#9)
 *
 *  The current protocol guarantees that all commands are processed in order
 *  exactly once. Requests are not guaranteed to process in order and may execute
 *  more than once so they should be idempotent.
 *
 *  The protocol also allows both sides to queue up requests and commands.
 *  In other words, it is possible to send multiple requests/commands at once
 *  without waiting for the individual acks.
 *
 *
 *  PABotBase Specifics:
 *
 *      -   PABotBase can queue up to 4 commands. If it receives any commands
 *          while the queue is full, it drops it and responds with
 *          "PABB_MSG_ERROR_COMMAND_DROPPED". (Any command that results in a
 *          button press or a wait is a long command.)
 *
 *      -   PABotBase can still handle other messages while it is running a long
 *          command.
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Protocol_H
#define PokemonAutomation_SerialPABotBase_Protocol_H

#include <stdbool.h>
#include <stdint.h>

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#elif __GNUC__
#define PABB_PACK   __attribute__((packed))
#else
#define PABB_PACK
#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define PABB_BAUD_RATE                  115200
#define PABB_PROTOCOL_OVERHEAD          (2 + sizeof(uint32_t))
#define PABB_PROTOCOL_MAX_PACKET_SIZE   64

//  Must be a power-of-two.
#define PABB_DEVICE_MINIMUM_QUEUE_SIZE  4

typedef uint32_t seqnum_t;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Basic Message Types

#define PABB_MSG_IS_ERROR(x)        (((x) & 0xf0) == 0x00)  //  Framework errors between 0x00 - 0x0f.
#define PABB_MSG_IS_ACK(x)          (((x) & 0xf0) == 0x10)  //  Acks between 0x10 - 0x1f.
#define PABB_MSG_IS_INFO(x)         (((x) & 0xe0) == 0x20)  //  Custom info between 0x20 - 0x3f.
#define PABB_MSG_IS_REQUEST(x)      (((x) & 0xc0) == 0x40)  //  Requests between 0x40 - 0x7f.
#define PABB_MSG_IS_COMMAND(x)      ((x) >= 0x80)           //  Commands between 0x80 - 0xff.

#define PABB_MSG_IS_REQUEST_OR_COMMAND(x)   ((x) >= 0x40)

////////////////////////////////////////////////////////////////////////////////
//  Framework Errors
#define PABB_MSG_ERROR_READY                    0x00
//  No Parameters

#define PABB_MSG_ERROR_INVALID_MESSAGE          0x01
typedef struct{
    uint8_t message_length;
} PABB_PACK pabb_MsgInfoInvalidMessage;

#define PABB_MSG_ERROR_CHECKSUM_MISMATCH        0x02
typedef struct{
    uint8_t message_length;
} PABB_PACK pabb_MsgInfoChecksumMismatch;

#define PABB_MSG_ERROR_INVALID_TYPE             0x03
typedef struct{
    uint8_t type;
} PABB_PACK pabb_MsgInfoInvalidType;

#define PABB_MSG_ERROR_INVALID_REQUEST          0x04
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgInfoInvalidRequest;

#define PABB_MSG_ERROR_MISSED_REQUEST           0x05
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgInfoMissedRequest;

#define PABB_MSG_ERROR_COMMAND_DROPPED          0x06
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgInfoCommandDropped;

#define PABB_MSG_ERROR_WARNING                  0x07
typedef struct{
    uint16_t error_code;
} PABB_PACK pabb_MsgInfoWARNING;

#define PABB_MSG_ERROR_DISCONNECTED             0x08
typedef struct{
    uint16_t error_code;
} PABB_PACK pabb_MsgInfoDisconnected;

////////////////////////////////////////////////////////////////////////////////
//  Ack

#define PABB_MSG_ACK_COMMAND                    0x10
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgAckCommand;

#define PABB_MSG_ACK_REQUEST                    0x11
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgAckRequest;

#define PABB_MSG_ACK_REQUEST_I8                 0x12
typedef struct{
    seqnum_t seqnum;
    uint8_t data;
} PABB_PACK pabb_MsgAckRequestI8;

#define PABB_MSG_ACK_REQUEST_I16                0x13
typedef struct{
    seqnum_t seqnum;
    uint16_t data;
} PABB_PACK pabb_MsgAckRequestI16;

#define PABB_MSG_ACK_REQUEST_I32                0x14
typedef struct{
    seqnum_t seqnum;
    uint32_t data;
} PABB_PACK pabb_MsgAckRequestI32;

#define PABB_MSG_ACK_REQUEST_DATA               0x1f
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgAckRequestData;

////////////////////////////////////////////////////////////////////////////////
//  Custom Info

#define PABB_MSG_INFO_I32                       0x20
typedef struct{
    uint8_t tag;
    uint32_t data;
} PABB_PACK pabb_MsgInfoI32;

#define PABB_MSG_INFO_DATA                      0x21
typedef struct{
    uint32_t tag;
} PABB_PACK pabb_MsgInfoData;

#define PABB_MSG_INFO_STRING                    0x23

#define PABB_MSG_INFO_I32_LABEL                 0x24
#define PABB_MSG_INFO_H32_LABEL                 0x25
typedef struct{
    uint32_t value;
} PABB_PACK pabb_MsgInfoI32Label;

////////////////////////////////////////////////////////////////////////////////
//  Static Requests

#define PABB_MSG_SEQNUM_RESET                   0x40
//  After you send this message, the next seqnum you should use is (seqnum + 1).
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgInfoSeqnumReset;

#define PABB_MSG_REQUEST_PROTOCOL_VERSION       0x41
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestProtocolVersion;

#define PABB_MSG_REQUEST_PROGRAM_VERSION        0x42
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestProgramVersion;

#define PABB_MSG_REQUEST_PROGRAM_ID             0x43
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestProgramID;

#define PABB_MSG_REQUEST_PROGRAM_NAME           0x44
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestProgramName;

#define PABB_MSG_REQUEST_CONTROLLER_LIST        0x45
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestControllerList;

#define PABB_MSG_REQUEST_QUEUE_SIZE             0x46
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestQueueSize;

////////////////////////////////////////////////////////////////////////////////
//  Mode Requests

#define PABB_MSG_REQUEST_READ_CONTROLLER_MODE   0x47
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestReadControllerMode;

#define PABB_MSG_REQUEST_CHANGE_CONTROLLER_MODE 0x48    //  Change controller and keep settings.
#define PABB_MSG_REQUEST_RESET_TO_CONTROLLER    0x49    //  Change controller and wipe settings. (unpair from host)
typedef struct{
    seqnum_t seqnum;
    uint32_t controller_id;
} PABB_PACK pabb_MsgRequestChangeControllerMode;

////////////////////////////////////////////////////////////////////////////////
//  Command Queue Requests

#define PABB_MSG_REQUEST_COMMAND_FINISHED       0x4a
//  When you receive this message, you must ack it with PABB_MSG_ACK_REQUEST.
typedef struct{
    seqnum_t seqnum;
    seqnum_t seq_of_original_command;
    uint32_t finish_time;
} PABB_PACK pabb_MsgRequestCommandFinished;

#define PABB_MSG_REQUEST_STOP                   0x4b
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestStop;

#define PABB_MSG_REQUEST_NEXT_CMD_INTERRUPT     0x4c
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_MsgRequestNextCmdInterrupt;

////////////////////////////////////////////////////////////////////////////////
//  Other Requests

#define PABB_MSG_REQUEST_CLOCK                  0x4f    //  Deprecated
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_system_clock;

#define PABB_MSG_REQUEST_STATUS                 0x50
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_Message_RequestStatus;

#define PABB_MSG_REQUEST_READ_MAC_ADDRESS       0x51
typedef struct{
    seqnum_t seqnum;
    uint32_t mode;
} PABB_PACK pabb_MsgRequestReadMacAddress;

////////////////////////////////////////////////////////////////////////////////
//  Commands

//  These are no longer supported by anything.

#if 0
#define PABB_MSG_COMMAND_SET_LED_STATE          0x81
typedef struct{
    seqnum_t seqnum;
    bool on;
} PABB_PACK pabb_MsgCommandSetLeds;
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#if _WIN32
#pragma pack(pop)
#endif

#endif
