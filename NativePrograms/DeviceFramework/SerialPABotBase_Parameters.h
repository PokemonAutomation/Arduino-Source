/*  Serial PABotBase Parameters
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SerialPABotBase_Parameters_H
#define PokemonAutomation_SerialPABotBase_Parameters_H


//  Protocol Version:
//
//      Backwards incompatible changes will increase by 100 or more.
//      Backwards compatible changes will increase by 1.
//
//      (version / 100) must be the same on both server and client.
//      (version % 100) may differ though client may apply restrictions.
//
#define PABB_PROTOCOL_VERSION           2023121900

//  Program versioning doesn't matter. It's just for informational purposes.
#define PABB_PROGRAM_VERSION            2023121900

#define PABB_RETRANSMIT_DELAY_MILLIS    80

#define PABB_MAX_MESSAGE_SIZE           12
#define PABB_MAX_PACKET_SIZE            (PABB_MAX_MESSAGE_SIZE + PABB_PROTOCOL_OVERHEAD)



//  This macro isn't part of the protocol. But it specifies the size of the
//  command queue on the device. This can be used as a hint by the client to
//  avoid overloading the device. This has no effect on correctness since the
//  device will automatically drop commands when the command queue fills up or
//  if it can't process serial messages quickly enough.

//  Must be a power-of-two.
#if __AVR_ATmega16U2__
#define PABB_DEVICE_QUEUE_SIZE          8
#elif __AVR_ATmega32U4__ || __AVR_AT90USB1286__
#define PABB_DEVICE_QUEUE_SIZE          64
#else
#define PABB_DEVICE_QUEUE_SIZE          4
#endif



#endif
