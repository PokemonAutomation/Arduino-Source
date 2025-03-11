/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H
#define PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FriendSearchDisconnect_Descriptor : public SingleSwitchProgramDescriptor{
public:
    FriendSearchDisconnect_Descriptor();
};



class FriendSearchDisconnect : public SingleSwitchProgramInstance{
public:
    FriendSearchDisconnect();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint8_t> USER_SLOT;
};




}
}
}
#endif

