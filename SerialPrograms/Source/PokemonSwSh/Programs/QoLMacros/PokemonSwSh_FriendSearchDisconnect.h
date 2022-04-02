/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H
#define PokemonAutomation_PokemonSwSh_FriendSearchDisconnect_H

#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FriendSearchDisconnect_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    FriendSearchDisconnect_Descriptor();
};



class FriendSearchDisconnect : public SingleSwitchProgramInstance{
public:
    FriendSearchDisconnect(const FriendSearchDisconnect_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    SimpleIntegerOption<uint8_t> USER_SLOT;
};




}
}
}
#endif

