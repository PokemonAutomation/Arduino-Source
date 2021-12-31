/*  General Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_Routines_H
#define PokemonAutomation_NintendoSwitch_Messages_Routines_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_Routines.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "NintendoSwitch_Commands_Routines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_close_game : public BotBaseRequest{
public:
    pabb_close_game params;
    DeviceRequest_close_game()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_CLOSE_GAME, params);
    }
};


}
}
#endif
