/*  Game Entry Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Messages_GameEntry_H
#define PokemonAutomation_PokemonSwSh_Messages_GameEntry_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_GameEntry.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "PokemonSwSh_Commands_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DeviceRequest_fast_reset_game : public BotBaseRequest{
public:
    pabb_fast_reset_game params;
    DeviceRequest_fast_reset_game(
        uint16_t start_game_mash, uint16_t start_game_wait,
        uint16_t enter_game_mash, uint16_t enter_game_wait
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.start_game_mash = start_game_mash;
        params.start_game_wait = start_game_wait;
        params.enter_game_mash = enter_game_mash;
        params.enter_game_wait = enter_game_wait;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_FAST_RESET_GAME, params);
    }
};
class DeviceRequest_settings_to_enter_game_den_lobby : public BotBaseRequest{
public:
    pabb_settings_to_enter_game_den_lobby params;
    DeviceRequest_settings_to_enter_game_den_lobby(
        bool tolerate_update_menu, bool fast,
        uint16_t enter_switch_pokemon_delay,
        uint16_t exit_switch_pokemon_delay
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.tolerate_update_menu = tolerate_update_menu;
        params.fast = fast;
        params.enter_switch_pokemon_delay = enter_switch_pokemon_delay;
        params.exit_switch_pokemon_delay = exit_switch_pokemon_delay;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY, params);
    }
};
class DeviceRequest_enter_game : public BotBaseRequest{
public:
    pabb_enter_game params;
    DeviceRequest_enter_game(
        bool backup_save, uint16_t enter_game_mash, uint16_t enter_game_wait
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.backup_save = backup_save;
        params.enter_game_mash = enter_game_mash;
        params.enter_game_wait = enter_game_wait;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ENTER_GAME, params);
    }
};




}
}
}
#endif
