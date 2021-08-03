/*  Program IDs for Serial Communication
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_ProgramIDs_H
#define PokemonAutomation_ProgramIDs_H

#include <stdint.h>



#define PABB_PID_UNSPECIFIED                                0x00
#define PABB_PID_CUSTOM_PROGRAM                             0x01
#define PABB_PID_SANDBOX                                    0x02
#define PABB_PID_PABOTBASE_12KB                             0x08
#define PABB_PID_PABOTBASE_31KB                             0x09



#ifdef __cplusplus

enum class PABotBaseLevel{
    NOT_PABOTBASE = 0,
    PABOTBASE_12KB = 1,
    PABOTBASE_31KB = 2,
};
inline PABotBaseLevel program_id_to_botbase_level(uint8_t id){
    switch (id){
    case PABB_PID_PABOTBASE_12KB:
        return PABotBaseLevel::PABOTBASE_12KB;
    case PABB_PID_PABOTBASE_31KB:
        return PABotBaseLevel::PABOTBASE_31KB;
    }
    return PABotBaseLevel::NOT_PABOTBASE;
}

#endif



#define PABB_PID_TURBOA                                     0x10
#define PABB_PID_MASS_RELEASE                               0x11
#define PABB_PID_FAST_CODE_ENTRY                            0x12
#define PABB_PID_SURPRISE_TRADE                             0x13
#define PABB_PID_TRADE_BOT                                  0x14
#define PABB_PID_CLOTHING_BUYER                             0x15
#define PABB_PID_BALL_THROWER                               0x16

#define PABB_PID_DATESPAM_WATT_FARMER                       0x20
#define PABB_PID_DATESPAM_BERRY_FARMER                      0x21
#define PABB_PID_DATESPAM_LOTO_FARMER                       0x22
#define PABB_PID_DATESPAM_STOW_ON_SIDE_FARMER               0x23
#define PABB_PID_DATESPAM_DAILY_HIGHLIGHT_FARMER            0x24

#define PABB_PID_MULTIGAME_FOSSIL                           0x30
#define PABB_PID_SHINYHUNT_REGI                             0x31
#define PABB_PID_SHINYHUNT_SWORDS_OF_JUSTICE                0x32
#define PABB_PID_SHINYHUNT_UNATTENDED_REGI                  0x33
#define PABB_PID_SHINYHUNT_UNATTENDED_SWORDS_OF_JUSTICE     0x34
#define PABB_PID_SHINYHUNT_UNATTENDED_STRONG_SPAWN          0x35
#define PABB_PID_SHINYHUNT_UNATTENDED_REGIGIGAS             0x36
#define PABB_PID_SHINYHUNT_UNATTENDED_REGIGIGAS2            0x37
#define PABB_PID_SHINYHUNT_UNATTENDED_IOA_TRADE             0x38
#define PABB_PID_CURRYHUNTER                                0x39

#define PABB_PID_BEAM_RESET                                 0x40
#define PABB_PID_EVENT_BEAM_FINDER                          0x41
#define PABB_PID_DAYSKIPPER_JPN                             0x42
#define PABB_PID_DAYSKIPPER_EU                              0x43
#define PABB_PID_DAYSKIPPER_US                              0x44
#define PABB_PID_DAYSKIPPER_JPN_7P8K                        0x45

#define PABB_PID_FRIEND_DELETE                              0x50
#define PABB_PID_DEN_ROLLER                                 0x51
#define PABB_PID_AUTOHOST_ROLLING                           0x52
#define PABB_PID_AUTOHOST_MULTIGAME                         0x53

#define PABB_PID_EGG_FETCHER2                               0x60
#define PABB_PID_EGG_HATCHER                                0x61
#define PABB_PID_EGG_COMBINED2                              0x62
#define PABB_PID_EGG_SUPER_COMBINED2                        0x63
#define PABB_PID_GOD_EGG_ITEM_DUPE                          0x64
#define PABB_PID_GOD_EGG_DUPLICATION                        0x65



#endif
