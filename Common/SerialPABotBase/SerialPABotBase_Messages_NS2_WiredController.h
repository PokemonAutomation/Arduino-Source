/*  SerialPABotBase Messages (Nintendo Switch 2 Wired Controller)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Messages_NS2_WiredController_H
#define PokemonAutomation_SerialPABotBase_Messages_NS2_WiredController_H

#include "../Controllers/NintendoSwitch2_WiredController_State.h"
#include "SerialPABotBase_Protocol.h"

#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#elif __GNUC__
#define PABB_PACK   __attribute__((packed))
#else
#define PABB_PACK
#endif


#ifdef __cplusplus
namespace PokemonAutomation{
namespace SerialPABotBase{
#endif


#define PABB_MSG_REPORT_NS2_WIRED_CONTROLLER    0x90
typedef struct{
    seqnum_t seqnum;
    uint16_t milliseconds;
    pa_NintendoSwitch2_WiredController_State report;
} PABB_PACK pabb_Message_Report_NS2_WiredController;



#ifdef __cplusplus
}
}
#endif


#if _WIN32
#pragma pack(pop)
#endif

#endif
