/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H
#define PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H

#include "CommonFramework/Options/SimpleInteger.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class FriendSearchDisconnect : public SingleSwitchProgram{
public:
    FriendSearchDisconnect();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint8_t> USER_SLOT;
};




}
}
}
#endif

